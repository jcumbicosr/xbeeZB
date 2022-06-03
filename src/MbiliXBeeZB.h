#ifndef MbiliXBeeZB_h
#define MbiliXBeeZB_h

/******************************************************************************
 * Includes
 ******************************************************************************/

#include <stdint.h>
#include <Arduino.h>

//Variable for debugging
#define	DEBUG_XBEE	2

#define PRINT_XBEE(str)	Serial.print(F("[XBEE] ")); Serial.print(str);
#define PRINTLN_XBEE(str)	Serial.print(F("[XBEE] ")); Serial.println(str);


//Different Max Sizes Used in Libraries
#define MAX_DATA			150
#define	MAX_PARSE			200

//Encryption Mode
#define ENCRYPT_MODE_ZIGBEE		0x00

//Different modes
#define UNICAST 	0
#define BROADCAST 	1
#define CLUSTER 	2

//Differents addressing types
#define	MY_TYPE		0
#define	MAC_TYPE	1

//Different RX frame types
#define _16B 		0 //only for XBee-802.15.4
#define _64B 		1 //only for XBee-802.15.4
#define NORMAL_RX 	2 // Receive packet --> AO=0
#define EXPLICIT_RX	3 // Explicit RX Indicator --> AO=1

//! Structure : used for storing the information needed to send or receive a
//! packet, such as the addresses and data
/*!
 */
struct packetXBee
{
  private:
  public:
  /************ IN ***********/
	//! Structure Variable : 32b Lower Mac Destination
	/*!
 	*/
	uint8_t macDL[4];

	//! Structure Variable : 32b Higher Mac Destination
	/*!
	 */
	uint8_t macDH[4];

	//! Structure Variable : Sending Mode:
	//! 0=UNICAST ; 1=BROADCAST ; 2=CLUSTER
	/*!
	 */
	uint8_t mode;

	//! Structure Variable : Address Type -> 0=16B ; 1=64B
	/*! Only available for XBee-802.15.4 in order to specify what sort of
	 *  addressing has been used: 16-b or 64-b addressing
	 */
	uint8_t address_type;

	//! Structure Variable : 16b Network Address Destination
	/*!
	 */
	uint8_t naD[2];

	//! Structure Variable : Data. All the data here, even when it is > payload
	/*!
	 */
	char data[MAX_DATA];

	//! Structure Variable : Real data length.
	/*!
	 */
	uint16_t data_length;

	//! Structure Variable : Source Endpoint (ZigBee)
	/*!
	 */
	uint8_t SD;

	//! Structure Variable : Destination Endpoint (ZigBee)
	/*!
	 */
	uint8_t DE;

	//! Structure Variable : Cluster Identifier (ZigBee)
	/*!
	 */
	uint8_t CID[2];

	//! Structure Variable : Profile Identifier (ZigBee)
	/*!
	 */
	uint8_t PID[2];

	//! Structure Variable : Specifies if Network Address is known:
	//! 0=unknown net address ; 1=known net address
	/*!
	 */
	uint8_t MY_known;

	//! Structure Variable : Sending options (depends on the XBee module)
	/*!
	 */
	uint8_t opt;


	/******** APLICATION *******/

	//! Structure Variable : 32b Lower Mac Source
	/*!
	 */
	uint8_t macSL[4];

	//! Structure Variable : 32b Higher Mac Source
	/*!
	 */
	uint8_t macSH[4];

	//! Structure Variable : 16b Network Address Source
	/*!
	 */
	uint8_t naS[2];

	//! Structure Variable : Receive Signal Strength Indicator
	/*!
	 */
	uint8_t RSSI;

	//! Structure Variable : Address Source Type
	/*! Only available for XBee-802.15.4 in order to specify what sort of
	 *  addressing has been used: 16-b or 64-b addressing
	 */
	uint8_t address_typeS;

	//! Structure Variable : Specifies the time when the first fragment of the
	//! packet was received
	/*!
	 */
	long time;


	/******** OUT **************/

	//! Structure Variable : Delivery Status
	/*!
	 */
	uint8_t deliv_status;

	//! Structure Variable : Discovery Status
	/*!
	 */
	uint8_t discov_status;

	//! Structure Variable : Network Address where the packet has been set
	/*!
	 */
	uint8_t true_naD[2];

	//! Structure Variable : Retries needed to send the packet
	/*!
	 */
	uint8_t retries;
};

//! Structure : rxPacket80_t
/*! This structure defines the XBee API frames related to RX data.
 * The Frame type for this sort frames is 0x80.
 */
struct rxPacket80_t
{
	uint8_t 	start;
	uint16_t 	length;
	uint8_t 	frameType;
	uint8_t 	macS[8];
	uint8_t 	rssi;
	uint8_t 	options;
	uint8_t		data[100];
	uint8_t		checksum;
};

//! Structure : rxPacket81_t
/*! This structure defines the XBee API frames related to RX data.
 * The Frame type for this sort frames is 0x81.
 */
struct rxPacket81_t
{
	uint8_t 	start;
	uint16_t 	length;
	uint8_t 	frameType;
	uint8_t 	naS[2];
	uint8_t 	rssi;
	uint8_t 	options;
	uint8_t		data[100];
	uint8_t		checksum;
};

//! Structure : rxPacket90_t
/*! This structure defines the XBee API frames related to RX data.
 * The Frame type for this sort frames is 0x90.
 */
struct rxPacket90_t
{
	uint8_t 	start;
	uint16_t 	length;
	uint8_t 	frameType;
	uint8_t 	macS[8];
	uint8_t 	reserved[2];
	uint8_t 	options;
	uint8_t		data[100];
	uint8_t		checksum;
};

//! Structure : rxPacket91_t
/*! This structure defines the XBee API frames related to RX data.
 * The Frame type for this sort frames is 0x91.
 */
struct rxPacket91_t
{
	uint8_t 	start;
	uint16_t 	length;
	uint8_t 	frameType;
	uint8_t 	macS[8];
	uint8_t 	reserved[2];
	uint8_t 	srcEndPoint;
	uint8_t 	dstEndPoint;
	uint16_t 	clusterID;
	uint16_t 	profileID;
	uint8_t 	options;
	uint8_t		data[100];
	uint8_t		checksum;
};

class MbiliXBeeZB
{
 
public:
	//! class constructor
  	/*!
	It does nothing
	\param void
	\return void
	 */
	MbiliXBeeZB()
    {
		// set the default maximum number of retries to '3'
		_send_retries = 3;
		
		// clear library buffers
		//memset( nodeID, 0x00, sizeof(nodeID) );
		//memset( linkKey, 0x00, sizeof(linkKey) );
	};
	
	//! It initializes the necessary variables
  	/*!
	It initalizes all the necessary variables
	\param uint32_t _uartb : specifies the uart rate 
	\return void
	 */
	void init();
	
	void init(uint32_t _uartb);
	
	/*
	* Function: Converts a string to an hex number
	*/
	uint8_t str2hex(char* str);
	
	//! It sets the destination parameters, such as the receiver address and
	//! the data to send
  	/*!
    \param packetXBee* paq : a packetXBee structure where some parameters should
			have been filled before calling this function. After this call, this
			structure is filled with the corresponding address and data
    \param char* address : the receiver MAC
    \param uint8_t* data : the data to send
    \param int length : length of the data field
    \param uint8_t type : origin identification type: MAC_TYPE or MY_TYPE
    \return '1' on success
     */
	int8_t setDestinationParams(	packetXBee* paq,
									const char* address,
									uint8_t* data,
									uint16_t length,
									uint8_t type	);
	
	//! It sends a packet from one XBee to another XBee in API mode
	/*! This function performs application-level retries.
	 * 	This function is only used for 64-bit addressing.
    \param char* macAddress : destination MAC address
    \param char* data : data to be sent (as string)
    \return '0' on success, '1' error
     */
    uint8_t send( char* macAddress, char* data );

    //! It sends a packet from one XBee to another XBee in API mode
	/*! This function performs application-level retries.
	 * 	This function is only used for 64-bit addressing.
    \param char* macAddress : destination MAC address
    \param uint8_t* pointer : pointer to buffer of data to be sent
    \param uint16_t length  : length of the buffer
    \return '0' on success, '1' error
     */
	uint8_t send( char* macAddress, uint8_t* pointer, uint16_t length );
	
	//! It gets the TX frame checksum
  	/*!
    \param uint8_t* TX : the pointer to the generated frame which checksum has
    to be calculated
	\return calculated checksum
	*/
	uint8_t getChecksum(uint8_t* TX);
	
	//! It generates the frame using eschaped characters
  	/*!
      \param struct packetXBee* _packet : packet for storing the data to send
      \param uint8_t* TX_array : array for storing the data
      \param uint8_t &protect: variable used for storing if some protected character
      \param uint8_t type: variable used for knowing the frame length
         */
      void gen_frame_ap2(struct packetXBee* _packet, uint8_t* TX_array, uint8_t &protect, uint8_t type);
	
	//! It generates the API frame when a TX is done
  	/*!
  	This function is used to generate the XBee data payload, which is composed
  	by the Application header and the final data field.
    \param struct packetXBee* _packet : packet for storing the data to send
    \param uint8_t* TX_array : array for storing the data
    \param uint8_t start_pos: start position
    */
    void genDataPayload(	struct packetXBee* _packet,
							uint8_t* TX_array,
							uint8_t start_pos	);
	
	//! It checks the checksum is good
  	/*!
    \param uint8_t* data_in : the string that contains the eschaped API frame AT command
    \param uint16_t end : the end of the frame
	\param uint16_t start : the start of the frame
	\return '0' if no error, '1' if error
	*/
	uint8_t checkChecksum(uint8_t* data_in, uint16_t end, uint16_t start);
	
	//! It parses the Modem Status message received by the XBee module
  	/*!
      \param uint8_t* data_in : the string that contains the eschaped API frame AT command
      \param uint16_t end : the end of the frame
      \param uint16_t start : the start of the frame
      \return '0' if no error, '1' if error
         */
      uint8_t modemStatusResponse(uint8_t* data_in, uint16_t end, uint16_t start);
	  
	//! It generates the correct API frame from an eschaped one
  	/*!
	\param uint8_t* data_in : the string that contains the eschaped API frame AT command
	\param uint16_t end : the end of the frame
	\param uint16_t start : the start of the frame
	\return '0' if no error, '1' if error
	*/
    void des_esc(uint8_t* data_in, uint16_t end, uint16_t start);
	
	//! It parses the ZB TX Status message received by the XBee module
  	/*!This function is used for all protocols, but the 802.15.4
      \param uint8_t* ByteIN : array to store the received answer
      \return '0' if no error, '1' if error
         */
      uint8_t txZBStatusResponse();

	//! It sends a packet to other XBee modules
  	/*!
    \param struct packetXBee* packet : the function gets the needed information 
			to send the packet from it
    \return '0' on success, '1' otherwise
    */
	uint8_t sendXBeePriv(struct packetXBee* packet);
	
	//! It sends a packet to others XBee modules
  	/*!
    \param struct packetXBee* packet : it is filled with the information needed
		to be able to send the packet
    \return '0' on success, '1' otherwise
     */
    uint8_t sendXBee(struct packetXBee* packet);
	
	//! This function receives a new xbee data packet
  	/*! If OK, the result is stored in _payload and _length
    \param uint32_t timeout : ms to wait until time-out before a packet arrives
    \return
		'6' --> ERROR: Error escaping character within payload bytes
		'5' --> ERROR: Error escaping character in checksum byte
		'4' --> ERROR: Checksum is not correct
		'3' --> ERROR: Checksum byte is not available
		'2' --> ERROR: Frame Type is not valid
		'1' --> ERROR: timeout when receiving answer
		'0' --> OK: The command has been executed with no errors
     */
    int8_t receivePacketTimeout( uint32_t timeout);
	
	
	/// Attributes /////////////////////////////////////////////////////////////
	
	//! Variable : delivery packet status
  	/*!
	 */
	uint8_t delivery_status;

	//! Variable : discovery process status
  	/*!
	 */
	uint8_t discovery_status;

	//! Variable : true 16b Network Address where the packet has been sent
  	/*!
	 */
	uint8_t true_naD[2];

	//! Variable : retries done during the sending
  	/*!
	 */
	uint8_t retries_sending;
	
	//! Variable : flag to indicate if a frame was truncated
  	/*!
	 */
	uint8_t frameNext;
	
	//! Variable : specifies if APS encryption is enabled or disabled
  	/*!
	 */
	uint8_t apsEncryption;
	
	//! Variable : encryption mode (ON/OFF) (0-1)
	/*!
	 */
	uint8_t encryptMode;
	
	//! Variable : It stores the last Modem Status indication received
	/*!
	 */
	uint8_t modem_status;
	
	//! Variable : It stores if the last sent packet has generated an error
	/*!
	 */
	int8_t error_TX;
	
	//! Variable : buffer for a received data packet
  	/*!
	 */
	uint8_t 	_payload[MAX_DATA];

	//! Variable : specifies the number of bytes in _payload
  	/*!
	 */
	uint16_t 	_length;
	
	//! Variable : specifies the source MAC Adddress when a packet is received
  	/*!
	 */
	uint8_t 	_srcMAC[8];
	
	//! Variable : specifies the source Network Adddress when a packet is received
  	/*!
	 */
	uint8_t 	_srcNA[2];
	
	//! Variable : specifies the RSSI received in the last packet
  	/*!
	 */
	int 	_rssi;
	
	//! Variable : specifies the maximum number of retries to be done
  	/*! If the sending process fails, up to _send_retries are done
	 */
	uint8_t _send_retries;
	
	
	
};


extern MbiliXBeeZB	xbeeZB;
#endif