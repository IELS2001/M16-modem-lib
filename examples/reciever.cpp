#include <Arduino.h>
#include <M16-lib.h>

#define RX_GPIO 32
#define TX_GPIO 33

M16 m16(UART_NUM_2);

void setup()
{
    Serial.begin(115200);
    m16.begin(RX_GPIO, TX_GPIO);
}

void loop()
{
    size_t len = m16.getRxBuffLenght();
    if (len > 0)
    {
        uint8_t data[len];
        m16.readRxBuff(data, len);
        Serial.print("Received data: ");
        for (size_t i = 0; i < len; i++)
        {
            Serial.println(data[i], HEX);
        }
    }
}