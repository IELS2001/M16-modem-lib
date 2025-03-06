#include "protocol.h"

using namespace coder_decoder;

int main()
{
	// std::cout << convertToBinary((unsigned char *)0b10010011) << std::endl;

	ProtocolStructure protocol;
	protocol.id = 0b101; // 5
	protocol.command = Command::TEST;
	protocol.data = 0b1111110101010101; // 341

	std::cout << "Decoded message: " << "ID: " << (unsigned short)protocol.id << "\tCommand: "
			  << (unsigned short)protocol.command << "\tData: " << (protocol.data & 0b0000001111111111) << std::endl;

	unsigned short codedMessage = code(protocol);
	std::cout << "Coded message:" << codedMessage << std::endl;

	ProtocolStructure decodedMessage = decode(codedMessage);
	std::cout << "Decoded message: " << "ID: " << (unsigned short)decodedMessage.id << "\tCommand: "
			  << (unsigned short)decodedMessage.command << "\tData: " << decodedMessage.data << std::endl;
	return 0;
}
