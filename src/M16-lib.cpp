/**
 * @file M16-lib.cpp
 * @brief Implementation of the M16 class.
 *
 * This file contains the implementation of the M16 class, which provides
 * an interface for interacting with the M16 modem using UART communication.
 *
 * @author Stian Østhus Lund
 * Modified by: Ole Anders Astad
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
		Serial.print("Failed to configure UART parameters for M16.");
	}
	if (uart_set_pin(this->uart_num, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) != ESP_OK)
	{
		Serial.print("Failed to set UART pins for M16.");
	}
	if (uart_driver_install(this->uart_num, 1024, 0, 0, NULL, 0) != ESP_OK)
	{
		Serial.print("Failed to install UART driver for M16.");
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

/**
 * @brief Sends a packet with the specified ID and data.
 *
 * This function packs the given ID and data into a 16-bit value and sends it
 * byte by byte using the sendByte method. The ID is shifted left by 13 bits
 * and combined with the data using a bitwise OR operation. Any remaining bits
 * in the uint8_t ID and uint16_t data will be truncated to fit into the 3-bit
 * and 13-bit fields respectively.
 *
 * @param ID The 3-bit identifier for the packet (0-7).
 * @param data The 13-bit data to be sent (0-8191).
 * @return bool Returns true if the packet was sent successfully, false otherwise.
 *         (Note: Error checking and return value implementation is pending).
 */
bool M16::sendPacket(uint8_t ID, uint16_t data)
{
	uint16_t packedData = (ID << 13) | data;
	uint8_t bytes[2];
	bytes[0] = packedData >> 8;
	bytes[1] = packedData & 0xff;
	uart_write_bytes(this->uart_num, (const char *)&bytes, 2);

	// TODO: Implement error checking and return value.
	return true;
}

void M16::sendByte_TEST(uint8_t byte)
{
	uart_write_bytes(this->uart_num, (const char *)&byte, 1);
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
