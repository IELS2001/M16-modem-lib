#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <iostream>

template <typename T>
std::string convertToBinary(T input);
// std::string convertToBinary(unsigned short input);
// std::string convertToBinary(unsigned char input);

namespace coder_decoder
{
	enum Command : unsigned char
	{
		SEND,
		TEST = 0b101,
	};

	struct ProtocolStructure
	{
		unsigned char id;
		Command command;
		unsigned short data;
	};

	unsigned short code(unsigned char id, Command command, unsigned short data);
	unsigned short code(ProtocolStructure send);
	ProtocolStructure decode(unsigned short messageToDecode);
} // namespace coder_decoder

#endif
