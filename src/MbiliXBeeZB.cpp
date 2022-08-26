#include "MbiliXBeeZB.h"

//! It initializes the necessary variables
/*!
It initalizes all the necessary variables
\param uint32_t _uartb : specifies the uart rate 
\return void
 */
void MbiliXBeeZB::init()
{
	return init(9600);
}

void MbiliXBeeZB::init( uint32_t _uartb)
{
	Serial1.begin(_uartb);
	delay(270);
	
	error_TX=2;
	encryptMode = ENCRYPT_MODE_ZIGBEE;
	apsEncryption=0;
	
	// wake up Xbee module in case Sleep Mode is activated
	

	frameNext=0;
	
}

/*
 * Function: Converts a string to an hex number
 * 
 */
uint8_t MbiliXBeeZB::str2hex(char* str)
{
    int aux=0, aux2=0;

    if( (*str>='0') && (*str<='9') )
    {
        aux=*str++-'0';
    }
    else if( (*str>='A') && (*str<='F') )
    {
        aux=*str++-'A'+10;
    }
    else if( (*str>='a') && (*str<='f') )
    {
        aux=*str++-'a'+10;
    }
    if( (*str>='0') && (*str<='9') )
    {
        aux2=*str-'0';
    }
    else if( (*str>='A') && (*str<='F') )
    {
        aux2=*str-'A'+10;
    }
    else if( (*str>='a') && (*str<='f') )
    {
        aux2=*str-'a'+10;
    }
    return aux*16+aux2;
}

/* Function: Sets the destination address and data to 'paq' structure
 * Parameters:
 * 	'paq' is a packetXBee structure where some parameters should have been filled
 * 		before calling this function. After this call, this structure is filled
 * 		with the corresponding address and data
 *  'address' is the receiver MAC
 *  'data' is the data to send
 *  'type' is the destination identifier type (MAC_TYPE or MY_TYPE)
 *
 * Returns '1' on success
 */
int8_t MbiliXBeeZB::setDestinationParams(	packetXBee* paq,
											const char* address,
											uint8_t* data,
											uint16_t length,
											uint8_t type	)
{
	uint8_t destination[8];
    uint8_t i=0;
    uint8_t j=0;
    char aux[2];
	uint16_t data_ind=0;

    // make sure the MAC address is defined with upper case letters
    strupr((char*)address);
	
	if( type==MAC_TYPE )
	{
		while(j<8)
		{
			aux[i-j*2]=address[i];
			aux[(i-j*2)+1]=address[i+1];
			destination[j]=str2hex(aux);
			i+=2;
			j++;
		}
            
		// set 64-b destination mac address
		paq->macDH[0] = destination[0];
		paq->macDH[1] = destination[1];
		paq->macDH[2] = destination[2];
		paq->macDH[3] = destination[3];
		paq->macDL[0] = destination[4];
		paq->macDL[1] = destination[5];
		paq->macDL[2] = destination[6];
		paq->macDL[3] = destination[7];  
            
		paq->address_type=_64B;
	}
	
	// fill data field until the end of the string
    for( uint16_t i=0 ; i < length ; i++ )
    {
        paq->data[data_ind]=data[i];
        data_ind++;

        // in the case MAX_DATA is reached, then data field is truncated
        if( data_ind>=MAX_DATA ) break;
    }

    // set data length with the actual counter value
    paq->data_length=data_ind;
	
	return 1;
}

/*
 * Function: Generates the RF Data field for a TX frame
 *
 * This function is used to generate the XBee data payload
 *
 * Parameters:
 * 	_packet : the packetXBee structure where the data to send is stored
 * 	TX_array : the array where the API frame is stored
 * 	start_pos : starting position in API frame
 *
 * Returns: Nothing
*/
void MbiliXBeeZB::genDataPayload(	struct packetXBee* _packet,
									uint8_t* TX_array,
									uint8_t start_pos	)
{

	// set data field
	for( uint16_t j=0 ; j<_packet->data_length ; j++)
	{
		TX_array[start_pos+j]=uint8_t(_packet->data[j]);
	}

}

/*
 * Function: Generates the escaped API frame when a TX is done
 *
 * Parameters:
 * 	_packet : the packetXBee structure where the data to send is stored
 * 	TX_array : the array where the API frame is stored
 * 	protect : specifies the number of chars that had been escaped
 * 	type : specifies the type of send
 *
 * Returns: Nothing
*/
void MbiliXBeeZB::gen_frame_ap2(	struct packetXBee* _packet,
									uint8_t* TX_array,
									uint8_t &protect,
									uint8_t type	)
{
	// define variables
    uint16_t index = 1;
    bool final = false;
    uint16_t aux = 0;
    uint16_t aux2 = 0;
	
	// Enter while loop until all bytes are done escaping if necessary
    while( index < ( _packet->data_length + type + protect) )
    {
        if( ( TX_array[index] == 0x11)
		||	( TX_array[index] == 0x13)
		||	( TX_array[index] == 0x7E)
		||	( TX_array[index] == 0x7D) )
        {
			TX_array[index] = TX_array[index] xor 0x20;
            protect++;
            aux = TX_array[index];
            TX_array[index] = 0x7D;
            uint16_t k = index - 1;
			
			// after including the escpaing indicator: 0x7D
            // include the escaped value and copy the rest of the buffer
            while( final == false )
            {
                aux2 = TX_array[k+2];
                TX_array[k+2] = aux;
                if( ( k + 3 ) >= ( _packet->data_length + type + protect ) )
                {
                    final = true;
                    break;
                }
                aux = TX_array[k+3];
                TX_array[k+3] = aux2;
                if( ((k+4)>=(_packet->data_length+type+protect)) )
                {
                    final = true;
                    break;
                }
                k++;
                k++;
            }
            final = false;
        }
		index++;
    }
}

/*
 * Function: Checks the checksum is good
 * Parameters:
 * 	data_in : the answer received by the module
 * 	end : the end of the frame
 * 	start : the start of the frame
 * Returns: Integer that determines if there has been any error 
 * 	error=2 --> The command has not been executed
 * 	error=1 --> There has been an error while executing the command
 * 	error=0 --> The command has been executed with no errors
*/
uint8_t MbiliXBeeZB::checkChecksum(uint8_t* data_in, uint16_t end, uint16_t start)
{	
    uint16_t checksum=0;	
		
    for(uint16_t it=3+start;it<end;it++)
    {
        checksum=checksum+data_in[it];
    }
    if( (checksum==255) ) return 0;
    checksum%=256;
    if( checksum!=255 ) return 1;
    return 0;
}

/*
 Function: Parses the Modem Status message received by the XBee module
 Parameters:
 	data_in : the answer received by the module
 	end : the end of the frame
 	start : the start of the frame
 Returns: Integer that determines if there has been any error
   error=2 --> The command has not been executed
   error=1 --> There has been an error while executing the command
   error=0 --> The command has been executed with no errors
*/
uint8_t MbiliXBeeZB::modemStatusResponse(uint8_t* data_in, uint16_t end, uint16_t start)
{
	// Check the checksum
    if(checkChecksum(data_in,end,start)) return 1;

    modem_status=data_in[start+4];
    return 0;
}

/*
 Function: Generates the correct API frame from an escaped one
 Parameters:
 	data_in : The string that contains the escaped API frame
 	end : the end of the frame
 	start : the start of the frame
 Returns: Nothing
*/
void MbiliXBeeZB::des_esc(uint8_t* data_in, uint16_t end, uint16_t start)
{
    uint16_t i=0;
    uint16_t aux=0;

    while( i<end )
    {
        while( data_in[start+i]!=0x7D && i<end ) i++;
        if( i<end )
        {
            aux=i+1;
            switch( data_in[start+i+1] )
            {
                case 0x31 : 	data_in[start+i]=0x11;
                break;
                case 0x33 : 	data_in[start+i]=0x13;
                break;
                case 0x5E : 	data_in[start+i]=0x7E;
                break;
                case 0x5D : 	data_in[start+i]=0x7D;
                break;
            }
            i++;
            end--;
            while( i<(end) ){
                data_in[start+i]=data_in[start+i+1];
                i++;
            }
            i=aux;
        }
    }
}

/*
 * Function: Parses the ZB TX Status message received by the XBee module
 *
 * Parameters:
 * 	data_in : the answer received by the module
 * 	end : the end of the frame
 * 	start : the start of the frame
 *
 * Returns: Integer that determines if there has been any error
 * 	error=2 --> The command has not been executed
 * 	error=1 --> There has been an error while executing the command
 * 	error=0 --> The command has been executed with no errors
*/
uint8_t MbiliXBeeZB::txZBStatusResponse()
{
	// create reception buffer
	uint8_t ByteIN[MAX_PARSE];
    unsigned long previous=millis();

    // set number of bytes that TX Status frame (0x8B) has
    uint16_t numberBytes=11;
    uint8_t end=0;
    uint16_t counter3=0;
    uint8_t undesired=0;
    uint8_t status=0;
    uint16_t num_TX=0;
    uint8_t num_esc=0;
    uint16_t interval=5000;
    uint8_t num_mes=0;
    uint16_t i=1;
    uint16_t length_mes=0;
    uint16_t length_prev=0;
    uint8_t maxFrame=80;

    error_TX=2;

    memset(ByteIN, 0x00, sizeof(ByteIN));
	
	// If a frame was truncated before, we set the first byte as 0x7E
	// and we add a new packet to 'num_mes' counter
    if( frameNext )
    {
        ByteIN[0]=0x7E;
        counter3=1;
        num_mes=1;
        frameNext=0;
    }
	
	// Read data from XBee while the following conditions are true:
	// - TX status is not received
	// - The maximum number of bytes are not received
	// - There is enough memory to store a whole new packet when a start
	//	 delimeter (0x7E) is received
	// - Timeout is not exceeded
    while( end==0 && !frameNext )
    {
		// check available data
		if( Serial1.available()>0 )
       	{
			// read byte from correspondent uart
       		ByteIN[counter3] = Serial1.read();
            counter3++;
            previous=millis();
			
			// check if a new frame is received
            if(ByteIN[counter3-1]==0x7E)
			{
				// if there is no memory available for a whole new packet
				// then we escape and select frameNext=1 in order to get it
				// the next time we read from XBee
           		if( (MAX_PARSE-counter3) < maxFrame )
           		{
					frameNext=1;
				}
                else num_mes++;
            }
			
			// If some corrupted frame has appeared, it is discarded,
            // counter3 is set to zero again
            if( (counter3==1) && (ByteIN[counter3-1]!=0x7E) )
            {
				counter3=0;
			}

			// if counter3 reaches the maximum data to parse, then finish
           	if( counter3>=MAX_PARSE )
           	{
				end=1;
			}
			
			// Discard any non-TX status frame which are determined by a frame
			// type which may be 0x8B(TX Status) or 0x8A(Modem Status)
           	if( (counter3==4+(uint16_t)status*6+undesired) && (undesired!=1) )
            {
            	if( (ByteIN[counter3-1]!= 0x8B) && (ByteIN[counter3-1]!=0x8A) )
				{
					// increment undesired counter
                	undesired=1;

                	// sum 3 new bytes corresponding to
	                // start delimiter (1Byte) + length (2Bytes)
                	numberBytes+=3;
                }
            }
			
			// if undesired counter is active, increment 'numberBytes'
            if( undesired==1 )
            {
				numberBytes++;
			}

            // If a escape character (0x7D) is found increment 'numberBytes'
            if( (ByteIN[counter3-1]==0x7D) && (!undesired) )
            {
            	numberBytes++;
            }
			
			/* If a modem status frame (0x8A)
             *  ____________________________________________________
             * |      |     |     |            |         |          |
             * | 0x7E | MSB | LSB | Frame Type | cmdData | checksum |
             * |______|_____|_____|____________|_________|__________|
             *    0      1     2        3          4          5
             */
            if( (ByteIN[counter3-1]==0x8A) && (counter3==(4+(uint16_t)status*6)) )
            {
				// increment in 6Bytes 'numberBytes'
            	numberBytes+=6;

            	// increment 'status' in order to add a new 'modem status frame'
            	status++;
            }
			
			// If a new frame is read after reading any undesired frame,
        	// decrement 'numberBytes' and set undesired
            if( (ByteIN[counter3-1]==0x7E) && (undesired==1) )
            {
            	numberBytes--;
				undesired=numberBytes-7;
            }

            // if 'counter3' is the same as 'numberBytes' we finish
        	// This means that TX status has been found
            if(counter3==numberBytes)
            {
            	end=1;
           	}
       	}
		
		// avoid millis overflow problem
		if( millis() < previous ) previous=millis();

		// check if time is out
        if( (millis()-previous) > interval )
       	{
        	end=1;
        	Serial1.flush();
        }
    }
	
	// Store number of read bytes
    num_TX=counter3;
    counter3=0;
	
	// If some corrupted frame has appeared we jump it
    if( ByteIN[0]!=0x7E )
    {
		// jump until a new frame start delimiter is found
		while( ByteIN[i]!=0x7E && i<num_TX )
        {
			i++;
		}
	}
	
	// Parse the received messages from the XBee
    while( num_mes>0 )
    {
		// get length of the packet until another start delimiter
		// is found or the end of read bytes is reached
        while( ByteIN[i]!=0x7E && i<num_TX )
        {
			i++;
		}
        length_mes=i-length_prev;
		
		// If any byte has been escaped, it must be converted before parsing it
		// So first count number of escaped bytes and then make conversion
        for( uint16_t it=0;it<length_mes;it++)
        {
            if( ByteIN[it+length_prev]==0x7D ) num_esc++;
        }

        // if there are escaped bytes, make conversion
        if( num_esc )
        {
			des_esc(ByteIN,length_mes,i-length_mes);
		}
		
		/* Call parsing function depending on the Frame Type
		 *  _______________________________________________
		 * |      |     |     |            |               |
		 * | 0x7E | MSB | LSB | Frame Type |    ......     |
		 * |______|_____|_____|____________|_______________|
		 *    0      1     2        3          variable
		 */
        switch( ByteIN[(i-length_mes)+3] )
        {
            case 0x8A :	//Modem Status
						modemStatusResponse( ByteIN,
											 length_mes-num_esc+length_prev,
											 i-length_mes);
						break;

            case 0x90 :	// Receive Packet (AO=0)
            case 0x91 :	// Explicit Rx Indicator (AO=1)
						//error_RX=rxData( ByteIN,
						//				 length_mes-num_esc+length_prev,
						//				 i-length_mes);
						#if DEBUG_XBEE > 1
						PRINTLN_XBEE(F("Ignore receive Packet"));
						#endif
						break;

            case 0x8B :	// Transmit Status
						true_naD[0]=ByteIN[i-length_mes+5];
						true_naD[1]=ByteIN[i-length_mes+6];
						retries_sending=ByteIN[i-length_mes+7];
						discovery_status=ByteIN[i-length_mes+9];
						delivery_status=ByteIN[i-length_mes+8];
						if( delivery_status==0 )
						{
							error_TX=0;
						}
						else
						{
							error_TX=1;
						}
						break;

            default   :	break;
        }
		
		// decrement number of pending packets to be treated
        num_mes--;

        // update previous index in input buffer in order to carry on with the
        // following message stored in 'memory'
        length_prev=i;
        i++;
        num_esc=0;
    }

    return error_TX;
}

/*
 * Function: Send a packet from one XBee to another XBee in API mode
 * 
 * Parameters: 
 *   packet :	A struct of packetXBee type
 * 
 * Returns: Integer that determines if there has been any error 
 * 	error=2 --> The command has not been executed
 * 	error=1 --> There has been an error while executing the command
 * 	error=0 --> The command has been executed with no errors
 * 
 * --> DIGI's XBee Packet inner structure:
 * 
 * StartDelimiter(1B) + Length(2B) +  Frame Data(variable) + Checksum(1B)
 *  ______________     ___________     __________________     __________
 * |              |   |     |     |   |					 |   |			|
 * |	0x7E	  | + | MSB | LSB | + |    Frame Data    | + |	1 Byte	|
 * |______________|   |_____|_____|   |__________________|   |__________|
 * 
*/
uint8_t MbiliXBeeZB::sendXBeePriv(struct packetXBee* packet)
{
	// Local variables
	uint8_t TX[120];
    uint8_t counter=0;  
    uint8_t protegido=0;
    uint8_t tipo=0;
    int8_t error=2;   
	
	error_TX=2;
    
    // clear TX variable where the frame is going to be filled
	memset(TX,0x00,sizeof(TX));
    
    // Create the XBee frame
    TX[0]=0x7E;
    TX[1]=0x00;
	
	if( (packet->mode==BROADCAST) || (packet->mode==UNICAST) )
    {
		// fragment length
        TX[2]=14+packet->data_length;         
        
        // set Frame Type
        TX[3]=0x10; 
		TX[4]=0x01; 
        tipo=18;
		
		if(packet->mode==BROADCAST)
        {
			// set BROADCAST address
            TX[5]=0x00;
			TX[6]=0x00;
			TX[7]=0x00;
			TX[8]=0x00;
			TX[9]=0x00;
			TX[10]=0x00;
			TX[11]=0xFF;
			TX[12]=0xFF;	
			
			// set reserved bytes
            TX[13]=0xFF;
            TX[14]=0xFE;
        }
		if(packet->mode==UNICAST)
        {
			// set chosen address in setDestinationParams function
			TX[5]=packet->macDH[0];
			TX[6]=packet->macDH[1];
			TX[7]=packet->macDH[2];
			TX[8]=packet->macDH[3];
			TX[9]=packet->macDL[0];
			TX[10]=packet->macDL[1];
			TX[11]=packet->macDL[2];
			TX[12]=packet->macDL[3];
			
			// set destination network address if known
            if (packet->MY_known==1) 
            {
				TX[13]=packet->naD[0];
                TX[14]=packet->naD[1];    
            }
            else
            {
                TX[13]=0xFF;
                TX[14]=0xFE;
            }
        }
		
		// set broadcast radius as 0x00, so it will be set to 
		// the maximum hops value (NH command)
        TX[15]=0x00;
        
        // set Options depending on the selected option 
        if( apsEncryption) TX[16]=0x20;
        else TX[16]=0x00;
		
		// generate RF Data payload which is composed by [Api header]+[Data]
		genDataPayload(packet,TX,17);
        
        // set checksum
        TX[packet->data_length+17]=getChecksum(TX);
    }
	else // CLUSTER Type
    {
		// not implement
		#if DEBUG_XBEE > 0
		PRINTLN_XBEE(F("CLUSTER Type not implement"));
		#endif
		return error;
	}
	
	// Generate the escaped API frame (it is necessary because AP=2)  
    gen_frame_ap2(packet,TX,protegido,tipo);
	
	// send frame through correspondent UART
    while(counter<(packet->data_length+tipo+protegido))
    {	
		// print byte through correspondent UART
		Serial1.write(TX[counter]);
        counter++;
    }
    
    counter=0;  
	
	// read XBee's answer to TX request	
	error_TX = txZBStatusResponse();
	error = error_TX; 
	
	packet->deliv_status=delivery_status;
    packet->discov_status=discovery_status;
    packet->true_naD[0]=true_naD[0];
    packet->true_naD[1]=true_naD[1];
    packet->retries=retries_sending;
   
    return error;
}



/*
 * Function: Send a packet from one XBee to another XBee in API mode
 *
 * Parameters:
 * 	'packet' : A struct of packetXBee type
 *
 * Returns: Integer that determines if there has been any error
 *	error=2 --> The command has not been executed
 *	error=1 --> There has been an error while executing the command
 *	error=0 --> The command has been executed with no errors
 *
 * --> DIGI's XBee Packet inner structure:
 *
 * StartDelimiter(1B) + Length(2B) +  Frame Data(variable) + Checksum(1B)
 *  ______________     ___________     __________________     __________
 * |              |   |     |     |   |                  |   |          |
 * |     0x7E     | + | MSB | LSB | + |    Frame Data    | + |  1 Byte  |
 * |______________|   |_____|_____|   |__________________|   |__________|
 *
 */
uint8_t MbiliXBeeZB::sendXBee(struct packetXBee* packet)
{
	uint16_t maxPayload=0;
    int8_t error=2;
	
	//case ZIGBEE:	
	if(encryptMode==ENCRYPT_MODE_ZIGBEE)
	{
		if(packet->mode==BROADCAST)
		{
			maxPayload=92;
		}
		else
		{
			maxPayload=84;
		}
	}
	else
	{
		if(packet->mode==BROADCAST)
		{
			if(apsEncryption) maxPayload=70;
			else maxPayload=74;
		}
		else
		{
			if(apsEncryption) maxPayload=62;
			else maxPayload=66;
		}
	}
	
	// Check if fragmentation is necessary due to packet length
    // is greater than maximum payload
    if(packet->data_length > maxPayload)
    {
		// Truncation needed
		packet->data_length=maxPayload;
		
		#if DEBUG_XBEE > 1
		PRINTLN_XBEE(F("Fragmentation of Packet"));
		#endif
    }


	/// send the prepared packet using the virtual function sendXBeePriv
	error = sendXBeePriv(packet);
	
	return error;
}

/*
 * Function: Send a packet from one XBee to another XBee in API mode
 * This function performs application-level retries.
 * This function is only used for 64-bit addressing.
 *
 * return:
 * 	'0' OK
 * 	'1' error
 */
uint8_t MbiliXBeeZB::send( char* macAddress, char* data )
{
	return send( macAddress, (uint8_t*)data, (uint16_t)strlen(data) );
}

/*
 * Function: Send a packet from one XBee to another XBee in API mode
 * This function performs application-level retries.
 * This function is only used for 64-bit addressing.
 *
 *
 * Return:
 * 	'0' OK
 * 	'1' error
 */
uint8_t MbiliXBeeZB::send( char* macAddress, uint8_t* pointer, uint16_t length )
{
	// counter for retries
	uint8_t counter = 0;
	const uint8_t MAX_RETRIES = _send_retries;
	
	// Pointer to an XBee packet structure
	packetXBee packet;

	// clear buffer
	memset( &packet, 0x00, sizeof(packet) );
	
	// Choose transmission mode: UNICAST or BROADCAST
	packet.mode = UNICAST;
	
	if( strlen(macAddress) == 16 )
	{
		// Set destination XBee parameters to packet
		setDestinationParams( &packet, macAddress, pointer, length, MAC_TYPE);
	}
	else
	{
		return 1;
	}
	
	// Send the packet until reach a successful transmision or reach
	// the specified number of retries	
	while( counter < MAX_RETRIES )
	{
		// Send XBee packet
		sendXBee( &packet );
		
		// Check TX flag
		if( error_TX == 0 )
		{
			// Send successful, exit the while loop
			return 0;
		}
		else
		{			
			// Error transmitting the packet
			// wait for a random delay between 100 and 500 ms
			delay( (unsigned long)rand()%400LU + 100LU);
		}
		
		// increment counter
		counter++;
	}
}

/*
 * Function: Calculates the checksum for a TX frame
 * Parameters:
 * 	TX : pointer to the frame whose checksum has to be calculated
 * Returns: the calculated checksum for the frame
*/
uint8_t MbiliXBeeZB::getChecksum(uint8_t* TX)
{
	uint8_t checksum=0;
	uint16_t length_field = (uint16_t)((TX[1]<<8)&0xFF00) + (uint16_t)(TX[2]&0x00FF);

	 // calculate checksum
    for( uint16_t i=0 ; i < length_field;i++)
    {
		checksum = (checksum + TX[i+3])&0xFF;
	}

	while( checksum > 255 )
	{
		checksum = checksum - 256;
	}
	checksum = 255 - checksum;

	return checksum;

}

/*
 * Function: receive a new xbee data packet
 *
 * Parameters:
 * 	timeout : milliseconds to wait until time-out before a packet arrives
 *
 * Returns: Integer that determines if there has been any error
 * 	'7' --> ERROR: Buffer full. not enough memory space
 * 	'6' --> ERROR: Error escaping character within payload bytes
 * 	'5' --> ERROR: Error escaping character in checksum byte
 * 	'4' --> ERROR: Checksum is not correct
 * 	'3' --> ERROR: Checksum byte is not available
 * 	'2' --> ERROR: Frame Type is not valid
 * 	'1' --> ERROR: timeout when receiving answer
 * 	'0' --> OK: The command has been executed with no errors
*/
int8_t MbiliXBeeZB::receivePacketTimeout( uint32_t timeout)
{
	uint32_t previous;
	uint16_t frame_length = 0;
	uint16_t packet_length = 0; // Packet length with escaped bytes converted
	uint16_t i = 0;
	uint8_t c;
	bool escape_pending = false;
	bool packet_completed = false;
	uint8_t frameType = 0;
	uint8_t buffer[MAX_DATA] = {0};

	// clear packet structure
	memset( _payload, 0x00, sizeof(_payload) );
	memset( buffer, 0x00, sizeof(buffer) );

	// init previous
	previous = millis();
	
	// Perform the different steps within a given timeout
	while( (millis()-previous) < timeout )
	{
		// Read char
		if( Serial1.available() )
		{
			c = Serial1.read();

			// Frame init detected
			if( c == 0x7E )
			{
				i = 0;
				escape_pending = false;
			}
			else if ( c == 0x7D )
			{
				escape_pending = true;
				continue;
			}
			else
			{
				if (escape_pending == true)
				{
					c = c xor 0x20;
					escape_pending = false;
				}
			}
			buffer[i++] = c;

			// Compute length
			if (i == 3) frame_length = (uint16_t)(buffer[1] << 8) + (uint16_t)c;

			// Check if we have a complete packet
			else if (i == (frame_length + 4))
			{
				packet_completed = true;
				packet_length = i;
				break;
			}
			// Check if there is still room
			else if (i >= sizeof(buffer)) return 7;
		}
	}
	
	// Check if we got a complete packet
	if (!packet_completed)  return 1;

	// We have a complete packet
	// Verify checksum
	if (getChecksum(buffer) != buffer[packet_length-1])  return 4;
	
		// Depending on frame type extract payload and
	frameType = buffer[3];

	// calculate paylaod length
	if( frameType == 0x80 )
	{
		rxPacket80_t* received = (rxPacket80_t*) buffer;

		// substract header size
		_length = packet_length - 15 ;

		// copy payload
		memcpy( _payload, received->data, _length);

		// copy source mac address
		memcpy( _srcMAC, received->macS, sizeof(_srcMAC));

		// get rssi level
		_rssi = (int)received->rssi*(-1);
	}
	else if (frameType == 0x81)
	{
		rxPacket81_t* received = (rxPacket81_t*) buffer;

		// substract header size
		_length = packet_length - 9 ;

		// copy payload
		memcpy( _payload, received->data, _length);

		// copy source Network Address (MY)
		memcpy( _srcNA, received->naS, sizeof(_srcNA));

		// get rssi level
		_rssi = (int)received->rssi*(-1);
	}
	else if (frameType == 0x90)
	{
		rxPacket90_t* received = (rxPacket90_t*) buffer;

		// substract header size
		_length = packet_length - 16 ;

		// copy payload
		memcpy( _payload, received->data, _length);

		// copy source mac address
		memcpy( _srcMAC, received->macS, sizeof(_srcMAC));
	}
	else if (frameType == 0x91)
	{
		rxPacket91_t* received = (rxPacket91_t*) buffer;

		// substract header size
		_length = packet_length - 22 ;

		// copy payload
		memcpy( _payload, received->data, _length);

		// copy source mac address
		memcpy( _srcMAC, received->macS, sizeof(_srcMAC));
	}
	else
	{
		// ERROR: frameType not valid
		return 2;
	}
	
	return 0;
	
}
	
MbiliXBeeZB	xbeeZB = MbiliXBeeZB();