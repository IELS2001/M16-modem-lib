/**
 * @file protocol.h
 * @brief Defines the communication protocol for encoding and decoding messages.
 *
 * This header file declares the functions and structures necessary for encoding and decoding
 * messages in a 16-bit format. It includes an enumeration for command types and a structure
 * to hold protocol data.
 * 
 * @author Ole Anders Astad
 * @date March 2025
 */

#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <iostream>

template <typename T>
std::string convertToBinary(T input);

namespace coder_decoder
{
	/**
	 * @brief Enumeration for command types used in message encoding.
	 *
	 * This enum represents the different command types that can be sent or received.
	 */
	enum Command : unsigned char
	{
		SEND,		  ///< Command for sending data.
		TEST = 0b101, ///< Command for testing purposes.
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
		unsigned char id;	 ///< Identification of the device (only first 3 bits used).
		Command command;	 ///< The command type indicating the action to perform.
		unsigned short data; ///< The actual data being transmitted.
	};

	unsigned short encode(unsigned char id, Command command, unsigned short data);
	unsigned short encode(ProtocolStructure send);
	ProtocolStructure decode(unsigned short messageToDecode);
} // namespace coder_decoder

#endif
