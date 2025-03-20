/**
 * @file protocol.cpp
 * @brief Provides encoding and decoding functionality for a custom communication protocol.
 *
 * This file contains functions to encode and decode messages using a structured format.
 * The protocol packs an ID, a command type, and data into a 16-bit message, allowing
 * efficient transmission and retrieval of information.
 * 
 * @author Ole Anders Astad
 * @date March 2025
 */
#include "protocol.h"

/**
 * @brief Converts an input value to a binary string representation.
 *
 * This function takes an input of type T and returns a string representation of its binary value.
 * The resulting string is prefixed with "0b" to indicate its binary format.
 *
 * @tparam T The type of the input parameter.
 * @param input The value to be converted into a binary string.
 * @return A string representing the binary value of the input.
 */
template <typename T>
std::string convertToBinary(T input)
{
	std::string output = "0b";
	for (int i = (sizeof(input) * 8) - 1; i >= 0; i--)
	{
		if (input & (1 << i))
		{
			output += '1';
		}
		else
		{
			output += '0';
		}
	}
	return output;
}

namespace coder_decoder
{
	/**
	 * @brief Encodes input values into a 16-bit message.
	 *
	 * This function constructs a 16-bit message by encoding an ID (3 bits), a command (3 bits),
	 * and data (10 bits). The format of the final message is:
	 * - bbb(ID)bbb(Command)bbbbbbbbbb(Data)
	 *
	 * @param id The ID to identify a unit (only the first 3 bits are kept).
	 * @param command The command/type of the action (only the first 3 bits are kept).
	 * @param data The actual data to send (first 10 bits are used).
	 * @return The final encoded message as an unsigned short.
	 */
	unsigned short coder_decoder::encode(unsigned char id, Command command, unsigned short data)
	{
		unsigned short codedMessage = 0;
		codedMessage |= ((0b00000111 & id) << (5 + 8));
		codedMessage |= ((0b00000111 & command) << (2 + 8));
		codedMessage |= 0b0000001111111111 & data;
		return codedMessage;
	}

	/**
	 * @brief Encodes a ProtocolStructure into a 16-bit message.
	 *
	 * This function takes a `ProtocolStructure` object and encodes its fields into a
	 * single 16-bit message.
	 *
	 * @param send The `ProtocolStructure` containing the ID, command, and data to encode.
	 * @return The final encoded message as an unsigned short.
	 */
	unsigned short encode(ProtocolStructure send)
	{
		return encode(send.id, send.command, send.data);
	}

	/**
	 * @brief Decodes a 16-bit message into a `ProtocolStructure`.
	 *
	 * This function extracts the ID (3 bits), command (3 bits), and data (10 bits) from the
	 * given 16-bit message and returns them in a `ProtocolStructure` object.
	 *
	 * @param messageToDecode The 16-bit encoded message.
	 * @return A `ProtocolStructure` containing the extracted ID, command, and data.
	 */
	ProtocolStructure coder_decoder::decode(unsigned short messageToDecode)
	{
		ProtocolStructure result;
		result.id = 0b0000000000000111 & (messageToDecode >> (5 + 8));
		result.command = static_cast<Command>(0b0000000000000111 & (messageToDecode >> (2 + 8)));
		result.data = (messageToDecode & 0b0000001111111111);
		return result;
	}
}
