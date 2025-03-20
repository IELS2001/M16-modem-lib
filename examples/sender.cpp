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
    m16.sendPacket(0x07, 0x1fff);    // Skal resultere i en pakke med to bytes (0xff og 0xff).
    vTaskDelay(pdMS_TO_TICKS(2000)); // Vent i 2 sekunder før vi sender igjen. (1.6s + margin)

    //  Kan bruke m16.sendByte_TEST(uint8_t byte); for å sende en enkelt byte direkte
    //  For eksempel:
    //  m16.sendByte_TEST(0xff);
    //  m16.sendByte_TEST(0xff);
    //  vTaskDelay(pdMS_TO_TICKS(2000));
}