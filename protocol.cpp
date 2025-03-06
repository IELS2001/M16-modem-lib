#include "protocol.h"

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
	/// @brief Converts the inputs to a 2 byte signal that can be sent. X are ignored 0 are used.
	/// @param id The ID to identify unit only the 3 first bits are kept XXXXX000
	/// @param command The command/type of the action only th 3 first bits are kept XXXXX000
	/// @param data The actual data we want to send we keep the first 10 bits XXXXXX00 00000000
	/// @return The final coded message to send 000(ID)000(command)0000000000(data)
	unsigned short coder_decoder::code(unsigned char id, Command command, unsigned short data)
	{
		unsigned short codedMessage = 0;
		codedMessage |= (id << (5 + 8));
		codedMessage |= (command << (2 + 8));
		codedMessage |= 0b0000001111111111 & data;
		return codedMessage;
	}

	unsigned short code(ProtocolStructure send)
	{
		return code(send.id, send.command, send.data);
	}

	ProtocolStructure coder_decoder::decode(unsigned short messageToDecode)
	{
		ProtocolStructure result;

		// unsigned short maxValueInBinary = 0b1111111111111111;

		result.id = 0b0000000000000111 & (messageToDecode >> (5 + 8));
		result.command = static_cast<Command>(0b0000000000000111 & (messageToDecode >> (2 + 8)));
		result.data = (messageToDecode & 0b0000001111111111);

		return result;
	}
}
