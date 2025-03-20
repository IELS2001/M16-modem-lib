#include <Arduino.h>
#include <M16-lib.h>

#define RX_GPIO 32
#define TX_GPIO 33

// Create an instance of M16. Pass the UART port we want to use to the constructor.
M16 m16(UART_NUM_2);

void setup()
{
    Serial.begin(115200);

    m16.begin(RX_GPIO, TX_GPIO);

    m16.requestReport();

    Serial.println("Report received:");
    Serial.println("Transport Block: " + String(m16.report.transportBlock));
    Serial.println("Bit Error Rate: " + String(m16.report.bitErrorRate));
    Serial.println("Signal Power: " + String(m16.report.signalPower));
    Serial.println("Noise Power: " + String(m16.report.noisePower));
    Serial.println("Packet Valid: " + String(m16.report.packetValid));
    Serial.println("Packed Invalid: " + String(m16.report.packedInvalid));
    Serial.println("Firmware Version: " + String(m16.report.firmwareVersion));
    Serial.println("Time Since Boot: " + String(m16.report.timeSinceBoot));
    Serial.println("Chip ID: " + String(m16.report.chipID));
    Serial.println("HW Rev: " + String(m16.report.hwRev));
    Serial.println("Channel: " + String(m16.report.channel));
    Serial.println("TB Valid: " + String(m16.report.tbValid));
    Serial.println("TX Complete: " + String(m16.report.txComplete));
    Serial.println("Diagnostic: " + String(m16.report.diagnostic));
    Serial.println("Power Level: " + String(m16.report.powerLevel));
}

void loop()
{
}