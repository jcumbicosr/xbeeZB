#include "src/MbiliXBeeZB.h"

// Define BROADCAST MAC address
//////////////////////////////////////////
char RX_ADDRESS[] = "000000000000FFFF";
//////////////////////////////////////////

// define variable
uint8_t error;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println(F("Receiving BROADCAST packets example"));

  xbeeZB.init();
}

void loop() {
  ///////////////////////////////////////////
  // 1. Receive packet
  ///////////////////////////////////////////  
  
  // receive XBee packet (wait for 10 seconds)
  error = xbeeZB.receivePacketTimeout( 10000 );

  // check answer  
  if( error == 0 ) 
  {
    // Show data stored in '_payload' buffer indicated by '_length'
    Serial.print(F("Data: "));  
    Serial.write( xbeeZB._payload, xbeeZB._length);
    Serial.println();
    
    // Show data stored in '_payload' buffer indicated by '_length'
    Serial.print(F("Length: "));  
    Serial.println( xbeeZB._length);
    
    // Show data stored in '_payload' buffer indicated by '_length'
    Serial.print(F("Source MAC Address: "));  
    PrintHex8( xbeeZB._srcMAC[0] );
    PrintHex8( xbeeZB._srcMAC[1] );
    PrintHex8( xbeeZB._srcMAC[2] );
    PrintHex8( xbeeZB._srcMAC[3] );
    PrintHex8( xbeeZB._srcMAC[4] );
    PrintHex8( xbeeZB._srcMAC[5] );
    PrintHex8( xbeeZB._srcMAC[6] );
    PrintHex8( xbeeZB._srcMAC[7] );
    Serial.println();    
    Serial.println(F("--------------------------------"));
  }  
  else
  {
    // Print error message:
    /*
     * '7' : Buffer full. Not enough memory space
     * '6' : Error escaping character within payload bytes
     * '5' : Error escaping character in checksum byte
     * '4' : Checksum is not correct    
     * '3' : Checksum byte is not available 
     * '2' : Frame Type is not valid
     * '1' : Timeout when receiving answer   
    */
    Serial.print(F("Error receiving a packet:"));
    Serial.println(error,DEC);     
    Serial.println(F("--------------------------------"));
  }

  ///////////////////////////////////////////
  // 2. Send packet
  ///////////////////////////////////////////  
  
  // send XBee packet
  error = xbeeZB.send( RX_ADDRESS, "hola mundo" );   
  
  // check TX flag
  if( error == 0 )
  {
    Serial.println(F("send ok"));
    
  }
  else 
  {
    Serial.println(F("send error"));
  }
  

}

void PrintHex8(uint8_t data) // prints 8-bit data in hex with leading zeroes
{
  if (data < 0x10) {Serial.print("0");} 
  Serial.print(data,HEX); 
}
