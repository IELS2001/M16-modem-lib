/**
 * @file M16-lib.h
 * @brief Header file for the M16 modem class.
 *
 * This file contains the declaration of the M16 class and associated structures,
 * which provide functions to interact with the M16 modem for serial communication.
 *
 * @author Stian Østhus Lund
 * @author Ole Anders Astad
 * @date March 2025
 */

#ifndef M16_LIB_H
#define M16_LIB_H

#include <Arduino.h>
#include <iostream>
#include "driver/uart.h"

#define M16_BAUD 9600

/*
Client: id(ID) Hei, til server (command) password (data)
Server: id(client ID) request data (command) no data (data)
Client: id(client ID) what sensor (command) sensor data (data) X sensor amount
Client: id(client ID) finished (command) no data (data)
Server: id(client ID) ok (command) sensor amount (data)
*/

// Max value is 7
enum Command : uint8_t
{
	HI,
	REQUEST_DATA,
	FINISHED,
	TEMP_SENSOR,
	PRESSURE_SENSOR,
	CONDUCTIVITY_SENSOR,
	PH_SENSOR,
	SENSOR_DATA_RECEIVED
};

/**
 * @brief Structure representing the components of the communication protocol.
 *
 * The `ProtocolStructure` contains an ID, a command type, and data. These components
 * are used to encode and decode messages for communication.
 *
 * @author Ole Anders Astad
 * @date March 2025
 */
struct ProtocolStructure
{
	unsigned char id = 0;	 ///< Identification of the device (only first 3 bits used).
	Command command = 0;	 ///< The command type indicating the action to perform.
	unsigned short data = 0; ///< The actual data being transmitted.
};

struct Report
{
	uint8_t startOfFrame = 0;
	uint16_t transportBlock = 0;
	uint8_t bitErrorRate = 0;
	uint8_t signalPower = 0;
	uint8_t noisePower = 0;
	uint16_t packetValid = 0;
	uint8_t packedInvalid = 0;
	uint8_t firmwareVersion = 0;
	uint32_t timeSinceBoot = 0;
	uint16_t chipID = 0;
	uint8_t hwRev = 0;
	uint8_t channel = 0;
	uint8_t tbValid = 0;
	uint8_t txComplete = 0;
	uint8_t diagnostic = 0;
	uint8_t reserved = 0;
	uint8_t powerLevel = 0;
	uint8_t reserved2 = 0;
	uint8_t endOfFrame = 0;
};

class M16
{
private:
	uart_port_t uart_num;
	void sendByte(uint8_t byte);
	bool sendPacket(unsigned short packet);
	unsigned short encode(unsigned char id, Command command, unsigned short data);
	unsigned short encode(ProtocolStructure send);

public:
	ProtocolStructure decode(unsigned short messageToDecode);
	ProtocolStructure decode(uint8_t *messageToDecode);
	Report report;
	M16(uart_port_t uart_num);
	void begin(uint8_t rx_pin, uint8_t tx_pin);
	void switchOperationMode();
	void setCommunicationChannel(uint8_t channel);
	void setPowerLevel(uint8_t powerLevel);
	bool requestReport();
	bool sendPacket(ProtocolStructure packet);
	bool sendPacket(unsigned char id, Command command, unsigned short data);
	size_t getRxBuffLength();
	int readRxBuff(uint8_t *data, size_t length);
};

template <typename T>
String convertToBinary(T input);

#endif // M16_LIB_H