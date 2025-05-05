/**
 * @file M16-lib.cpp
 * @brief Implementation of the M16 class.
 *
 * This file contains the implementation of the M16 class, which provides
 * an interface for interacting with the M16 modem using UART communication.
 *
 * @author Stian Østhus Lund
 * @author Ole Anders Astad
 * @date March 2025
 */
#include "M16-lib.h"

/**
 * @brief Constructor for the M16 class.
 *
 * This constructor initializes an M16 object with a reference to a uart_port_t object.
 *
 * @param uart_num A reference to a uart_port_t object used for serial communication.
 */
M16::M16(uart_port_t uart_num) : uart_num(uart_num) {}

/**
 * @brief Initializes the M16 module with the specified RX and TX pins.
 *
 * This function sets up the serial communication for the M16 module using the
 * specified RX and TX pins and a predefined baud rate and serial configuration.
 *
 * @param rx_pin The pin number to be used for receiving data (RX).
 * @param tx_pin The pin number to be used for transmitting data (TX).
 */
void M16::begin(uint8_t rx_pin, uint8_t tx_pin)
{
	uart_config_t uart_config = {
		.baud_rate = M16_BAUD,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
		.rx_flow_ctrl_thresh = 122,
	};
	if (uart_param_config(this->uart_num, &uart_config) != ESP_OK)
	{
		Serial.println("Failed to configure UART parameters for M16.");
	}
	if (uart_set_pin(this->uart_num, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) != ESP_OK)
	{
		Serial.println("Failed to set UART pins for M16.");
	}
	if (uart_driver_install(this->uart_num, 1024, 0, 0, NULL, 0) != ESP_OK)
	{
		Serial.println("Failed to install UART driver for M16.");
	}
}

/**
 * @brief Sends a single byte of data through the M16 modem.
 *
 * This function sends a single byte through the M16 modem.
 *
 * @param byte The byte of data to be sent.
 */
void M16::sendByte(uint8_t byte)
{
	uart_write_bytes(this->uart_num, (const char *)&byte, 1);
}

bool M16::sendPacket(unsigned short packet)
{
	uint8_t bytes[2];
	bytes[0] = (packet >> 8) & 0xff;
	bytes[1] = packet & 0xff;
#ifdef DEBUG
	Serial.printf("Byte[0] from packet %s\n", convertToBinary(bytes[0]));
	Serial.printf("Byte[1] from packet %s\n", convertToBinary(bytes[1]));
#endif
	uart_write_bytes(this->uart_num, (const char *)&bytes, 2);

	// TODO: Implement error checking and return value.
	return true;
}

/**
 * @brief Switches the operation mode of the M16 device.
 *
 * This function sends a specific byte (0x6d) to the device to initiate
 * a mode switch. It then waits for 1000 milliseconds before sending
 * the byte again to complete the mode switch process.
 *
 * The modem boots into Transparent Mode by default.
 */
void M16::switchOperationMode()
{
	this->sendByte(0x6d);
	vTaskDelay(pdMS_TO_TICKS(1000));
	this->sendByte(0x6d);
}

/**
 * @brief Sets the communication channel for the M16 device.
 *
 * This function sets the communication channel for the M16 device by sending
 * a series of bytes to the device.
 *
 * @param channel The communication channel to set (must be between 1 and 12).
 */
void M16::setCommunicationChannel(uint8_t channel)
{
	if (channel < 1 || channel > 12)
	{
		// Invalid channel, do nothing.
		Serial.printf("Wrong channel\n");
		return;
	}

	// Send the channel change command.
	this->sendByte(0x63);
	vTaskDelay(pdMS_TO_TICKS(1000));
	this->sendByte(0x63);
	vTaskDelay(pdMS_TO_TICKS(1));

	// Send the channel character.
	if (channel <= 9)
	{
		this->sendByte(0x30 + channel); // Channels 1-9
	}
	else
	{
		this->sendByte(0x61 + (channel - 10)); // Channels 10-12 ('a', 'b', 'c')
	}
}

/**
 * @brief Sets the power level of the M16 device.
 *
 * This function sets the power level of the M16 device by sending the appropriate
 * command and power level character. The power level must be between 1 and 4 inclusive.
 *
 * @param powerLevel The desired power level (must be between 1 and 4).
 */
void M16::setPowerLevel(uint8_t powerLevel)
{
	if (powerLevel < 1 || powerLevel > 4)
	{
		// Invalid power level, do nothing.
		return;
	}

	// Set power level command.
	this->sendByte(0x6c);
	vTaskDelay(pdMS_TO_TICKS(1000));
	this->sendByte(0x6c);
	vTaskDelay(pdMS_TO_TICKS(1500));

	// Send the power level character.
	this->sendByte(0x30 + powerLevel);
}

/**
 * @brief Requests a report from the M16 device.
 *
 * This function sends a request byte to the M16 device and waits for the report to be available.
 * It retries the request up to 100 times with a delay of 10 milliseconds between each retry.
 * If the report is available, it reads the report data into the report struct.
 *
 * @return true if the report is successfully received, false if the request times out.
 */
bool M16::requestReport()
{
	this->sendByte(0x72);
	vTaskDelay(pdMS_TO_TICKS(1000));
	this->sendByte(0x72);

	uint8_t reportBytes[18];
	uint8_t retries = 0;
	size_t bytesRead = 0;

	// Wait until the report is available.
	while (bytesRead < sizeof(reportBytes))
	{
		int result = uart_read_bytes(this->uart_num, reportBytes + bytesRead, sizeof(reportBytes) - bytesRead, pdMS_TO_TICKS(10));
		if (result > 0)
		{
			bytesRead += result;
		}
		else
		{
			if (retries++ > 100)
			{
				return false;
			}
		}
	}
	this->report.startOfFrame = reportBytes[0];
	this->report.transportBlock = (reportBytes[1] << 8) | reportBytes[2];
	this->report.bitErrorRate = reportBytes[3];
	this->report.signalPower = reportBytes[4];
	this->report.noisePower = reportBytes[5];
	this->report.packetValid = (reportBytes[6] << 8) | reportBytes[7];
	this->report.packedInvalid = reportBytes[8];
	this->report.firmwareVersion = reportBytes[9];
	this->report.timeSinceBoot = (reportBytes[10] << 16) | (reportBytes[11] << 8) | reportBytes[12];
	this->report.chipID = (reportBytes[13] << 8) | reportBytes[14];
	this->report.hwRev = (reportBytes[15] & 0b00000011);
	this->report.channel = (reportBytes[15] & 0b00111100) >> 2;
	this->report.tbValid = (reportBytes[15] & 0b01000000) >> 6;
	this->report.txComplete = (reportBytes[15] & 0b10000000) >> 7;
	this->report.diagnostic = (reportBytes[16] & 0b00000001);
	// this->report.reserved = (reportBytes[16] & 0x02) >> 1;
	this->report.powerLevel = (reportBytes[16] & 0b00001100) >> 2;
	// this->report.reserved2 = (reportBytes[16] >> 0 & 0xf0) >> 4;
	this->report.endOfFrame = reportBytes[17];

	return true;
}

bool M16::sendPacket(ProtocolStructure packet)
{
	unsigned short encodedPackage = encode(packet);
#ifdef DEBUG
	Serial.print("Encoded data: ");
	Serial.println(convertToBinary(encodedPackage));
#endif
	return sendPacket(encodedPackage);
}

bool M16::sendPacket(unsigned char id, Command command, unsigned char data)
{
	unsigned short encodedPackage = encode(id, command, data);
	return sendPacket(encodedPackage);
}

size_t M16::getRxBuffLength()
{
	size_t buffered_size;
	uart_get_buffered_data_len(this->uart_num, &buffered_size);
	return buffered_size;
}

int M16::readRxBuff(uint8_t *data, size_t length)
{
	int num = 0;
	num = uart_read_bytes(this->uart_num, data, length, pdMS_TO_TICKS(100));
	uart_flush_input(this->uart_num);
	return num;
}

/**
 * @brief Encodes input values into a 16-bit message.
 *
 * This function constructs a 16-bit message by encoding an ID (4 bits), a command (4 bits),
 * and data (8 bits). The format of the final message is:
 * - bbbb(ID)bbbb(Command)bbbbbbbb(Data)
 *
 * @param id The ID to identify a unit (only the first 3 bits are kept).
 * @param command The command/type of the action (only the first 3 bits are kept).
 * @param data The actual data to send (first 10 bits are used).
 * @return The final encoded message as an unsigned short.
 */
unsigned short M16::encode(unsigned char id, Command command, unsigned char data)
{
	unsigned short codedMessage = 0;
	codedMessage |= ((0b00001111 & id) << (4 + 8));
	codedMessage |= ((0b00001111 & command) << (8));
	codedMessage |= 0b0000000011111111 & data;
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
unsigned short M16::encode(ProtocolStructure send)
{
	return encode(send.id, send.command, send.data);
}

/**
 * @brief Decodes a 16-bit message into a `ProtocolStructure`.
 *
 * This function extracts the ID (4 bits), command (4 bits), and data (8 bits) from the
 * given 16-bit message and returns them in a `ProtocolStructure` object.
 *
 * @param messageToDecode The 16-bit encoded message.
 * @return A `ProtocolStructure` containing the extracted ID, command, and data.
 */
ProtocolStructure M16::decode(unsigned short messageToDecode)
{
	ProtocolStructure result{0, Command::HI, 0};
	result.id = 0b0000000000001111 & (messageToDecode >> (4 + 8));
	result.command = static_cast<Command>(0b0000000000001111 & (messageToDecode >> (8)));
	result.data = (messageToDecode & 0b0000000011111111);
	return result;
}

ProtocolStructure M16::decode(uint8_t *messageToDecode)
{
	ProtocolStructure result{0, Command::HI, 0};
	result.id = 0b00001111 & (messageToDecode[0] >> 4);
	result.command = static_cast<Command>(0b00001111 & (messageToDecode[0]));
	result.data = messageToDecode[1];
	return result;
}

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
String convertToBinary(T input)
{
	String output = "0b";
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
