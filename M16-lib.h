/**
 * @file M16-lib.h
 * @brief Header file for the M16 modem class.
 *
 * This file contains the declaration of the M16 class and associated structures,
 * which provide functions to interact with the M16 modem for serial communication.
 *
 * @author Stian Østhus Lund
 * @date March 2025
 */

#ifndef M16_LIB_H
#define M16_LIB_H

#include <Arduino.h>
#include "driver/uart.h"

#define M16_BAUD 9600

struct Report
{
    uint8_t startOfFrame;
    uint16_t transportBlock;
    uint8_t bitErrorRate;
    uint8_t signalPower;
    uint8_t noisePower;
    uint16_t packetValid;
    uint8_t packedInvalid;
    uint8_t firmwareVersion;
    uint32_t timeSinceBoot;
    uint16_t chipID;
    uint8_t hwRev;
    uint8_t channel;
    uint8_t tbValid;
    uint8_t txComplete;
    uint8_t diagnostic;
    uint8_t reserved;
    uint8_t powerLevel;
    uint8_t reserved2;
    uint8_t endOfFrame;
};

class M16
{
private:
    uart_port_t uart_num;
    void sendByte(uint8_t byte);
    void readUartByte();
    void getRxLenght();

public:
    Report report;
    M16(uart_port_t uart_num);
    void begin(uint8_t rx_pin, uint8_t tx_pin);
    void switchOperationMode();
    void setCommunicationChannel(uint8_t channel);
    void setPowerLevel(uint8_t powerLevel);
    bool requestReport();
    bool sendPacket(uint8_t ID, uint16_t data);
    size_t getRxBuffLenght();
    int readRxBuff(uint8_t *data, size_t length);
    void sendByte_TEST(uint8_t byte);
};

#endif // M16_LIB_H
