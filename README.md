# XBee ZigBee Library for Arduino

A comprehensive Arduino library for communicating with XBee ZigBee modules using API mode. This library provides an easy-to-use interface for sending and receiving packets, managing sleep modes, and handling various XBee operations.

## Features

- **API Mode Communication**: Full support for XBee API mode operations
- **Packet Transmission**: Send unicast and broadcast packets
- **Packet Reception**: Receive packets with timeout functionality
- **Sleep Management**: Put XBee modules to sleep and wake them up
- **Address Handling**: Support for 64-bit MAC addresses
- **Error Handling**: Comprehensive error reporting and status checking
- **Broadcast Support**: Receive and send broadcast packets
- **RSSI Monitoring**: Signal strength indication for received packets

## Hardware Requirements

- Arduino board (Uno, Mega, etc.)
- XBee ZigBee module (Series 2)
- Appropriate XBee shield or breakout board
- Serial connection between Arduino and XBee

## Installation

1. Download or clone this repository
2. Copy the `src/` folder contents to your Arduino libraries folder
3. Include the library in your Arduino sketch: `#include "src/MbiliXBeeZB.h"`

## Usage

### Basic Setup

```cpp
#include "src/MbiliXBeeZB.h"

// Define destination address (broadcast example)
char RX_ADDRESS[] = "000000000000FFFF";

void setup() {
  Serial.begin(9600);
  Serial.println(F("XBee ZigBee Communication Example"));

  xbeeZB.ON();  // Initialize XBee
  xbeeZB.wake();  // Wake up XBee if in sleep mode
}

void loop() {
  // Your communication logic here
}
```

### Receiving Packets

```cpp
uint8_t error;

// Receive packet with 10 second timeout
error = xbeeZB.receivePacketTimeout(10000);

if (error == 0) {
  // Packet received successfully
  Serial.print(F("Data: "));
  Serial.write(xbeeZB._payload, xbeeZB._length);
  Serial.println();

  Serial.print(F("Length: "));
  Serial.println(xbeeZB._length);

  Serial.print(F("Source MAC: "));
  for(int i = 0; i < 8; i++) {
    if(xbeeZB._srcMAC[i] < 0x10) Serial.print("0");
    Serial.print(xbeeZB._srcMAC[i], HEX);
  }
  Serial.println();
} else {
  // Handle error
  Serial.print(F("Receive error: "));
  Serial.println(error, DEC);
}
```

### Sending Packets

```cpp
uint8_t error;

// Send packet to broadcast address
error = xbeeZB.send(RX_ADDRESS, "Hello World");

if (error == 0) {
  Serial.println(F("Packet sent successfully"));
} else {
  Serial.println(F("Send error"));
}
```

### Sleep Management

```cpp
// Put XBee to sleep
xbeeZB.sleep();

// Wake up XBee
xbeeZB.wake();
```

## API Reference

### Core Methods

#### `void ON()`
Initializes the XBee module with default baud rate (9600).

#### `void ON(uint32_t baudrate)`
Initializes the XBee module with specified baud rate.

#### `uint8_t send(char* address, char* data)`
Sends a string message to the specified MAC address.
- **Parameters**:
  - `address`: Destination MAC address as string
  - `data`: Message to send
- **Returns**: 0 on success, error code otherwise

#### `uint8_t send(char* address, uint8_t* data, uint16_t length)`
Sends binary data to the specified MAC address.
- **Parameters**:
  - `address`: Destination MAC address as string
  - `data`: Pointer to data buffer
  - `length`: Length of data
- **Returns**: 0 on success, error code otherwise

#### `int8_t receivePacketTimeout(uint32_t timeout)`
Receives a packet with specified timeout.
- **Parameters**:
  - `timeout`: Timeout in milliseconds
- **Returns**: 0 on success, error code otherwise

#### `void wake()`
Wakes up the XBee module from sleep mode.

#### `void sleep()`
Puts the XBee module into sleep mode.

### Public Variables

#### `uint8_t _payload[MAX_DATA]`
Buffer containing received packet data.

#### `uint16_t _length`
Length of data in the payload buffer.

#### `uint8_t _srcMAC[8]`
Source MAC address of the last received packet.

#### `uint8_t _srcNA[2]`
Source network address of the last received packet.

#### `int _rssi`
Received Signal Strength Indicator of the last packet.

## Error Codes

### Receive Errors
- `1`: Timeout when receiving answer
- `2`: Frame Type is not valid
- `3`: Checksum byte is not available
- `4`: Checksum is not correct
- `5`: Error escaping character in checksum byte
- `6`: Error escaping character within payload bytes
- `7`: Buffer full. Not enough memory space

### Send Errors
- `0`: Success
- `1`: Error

## Example Projects

### Broadcast Receiver
The included `xbeeZB.ino` demonstrates:
- Receiving broadcast packets
- Displaying packet information (data, length, source MAC)
- Conditional sleep/wake based on received data
- Sending response packets

### Key Features in Example
- Broadcast address: `000000000000FFFF`
- Sleep trigger: Receiving "SLP" command
- Automatic response sending

## Configuration

### XBee Module Settings
Ensure your XBee modules are configured for API mode:
- `AP=2` (API mode with escaped characters)
- `SM=1` (Pin hibernate for sleep mode)

### Arduino Pin Connections
- XBee TX → Arduino RX (typically pin 0 for Serial, pin 19 for Serial1 on Mega)
- XBee RX → Arduino TX (typically pin 1 for Serial, pin 18 for Serial1 on Mega)
- XBee Sleep Pin → Arduino pin 9 (if using sleep functionality)

## Troubleshooting

### Common Issues
1. **No packets received**: Check XBee configuration and baud rates
2. **Send failures**: Verify destination address format
3. **Sleep mode issues**: Ensure SM=1 on XBee and proper pin connections

### Debug Output
Enable debug output by setting `DEBUG_XBEE` in the header file.


## Acknowledgments

This library is based on the Waspmote implementation, providing a robust interface for XBee communication in Arduino projects.