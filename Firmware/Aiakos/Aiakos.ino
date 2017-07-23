#include "loradevice.h"

/* Hardware Connections
 * **************************************************
 * LoRa module
 * **************************************************
 * XL1276    Protrinket  Due    Nucleo   Blue Pill
 *           3V
 *****************************************************
 * NSS       10          4       A2     PA4
 * MOSI      11          ICSP.4  D11    PA7
 * MISO      12          ICSP.1  D12    PA6
 * SCK       13          ICSP.3  D13    PA5 (Be careful on Nucleo, this is the 6th pin on the left row of the right most pin header connector, not the fifth!)
 * REST      RST         RESET   NRST   R
 * DIO0      3           3       D5     PA12
 * VCC       3V          3.3V    3V     3.3
 * GND       G           GND     GND    G
 *
 *****************************************************
 * ATSHA204A for TRNG and unique serial number (bitbanged-I²C)
 *****************************************************
                Blue Pill   Nucleo   ATSHA204A
 *****************************************************
 *              PB9         D3      5 (SDA)
 *              PB8         D4      6 (SCL)
 *
 *****************************************************
 * Serial connection for secure pairing
 *                      Due     Nucleo          Blue Pill
 *****************************************************
 * Tip                   TX1     D2(UART1_RX)   PB11 (UART3_RX)
 * Ring                  RX1     D8(UART1_TX)   PB10 (UART3_TX)
 * Sleeve                GND     GND            G
 * Cable detect Contact  2       D6             PB1
 * Pushbutton                    25             PA11
 *
 *****************************************************
 * Pulse output for opening/closing garage door
 *****************************************************
 * Pulse                 5
 *
 * Pairing procedure:
 * ------------------
 * Interconnect the keyfob and the garage controller with the stereo jack cable.  Press the blue button on the Nucleo
 * to start the pairing.
 *
 * Authenticating procedure:
 * -------------------------
 * Disconnect the stereo jack cable from the key fob as well as from the garage controller.  Press the blue button on
 * the Nucleo to initiate authentication.
*/


#ifdef ARDUINO_STM_NUCLEO_F103RB
#define ROLE_KEYFOB
#elif defined(ARDUINO_GENERIC_STM32F103C)
#define ROLE_KEYFOB
#elif defined(ARDUINO_SAM_DUE)
#define ROLE_GARAGE_CONTROLLER
#else
#error No device type defined.
#endif

namespace
{
Configuration cfg;
LoRaDevice* ld;
}

#ifdef ROLE_GARAGE_CONTROLLER
#include "garagecontroller.h"
namespace
{
RH_RF95 rhLoRa(4,3);
RH_Serial rhSerial(Serial1);
GarageController device(1, &cfg, &rhLoRa, &rhSerial, 2);
}
#elif defined(ROLE_KEYFOB)
#include "keyfob.h"
namespace
{
#ifdef ARDUINO_STM_NUCLEO_F103RB
RH_Serial rhSerial(Serial1);
RH_RF95 rhLoRa(A2,5);//NSS, DIO0
KeyFob device(2, &cfg, &rhLoRa, &rhSerial, 25, 6);
#elif defined(ARDUINO_GENERIC_STM32F103C)
RH_Serial rhSerial(Serial2);
RH_RF95 rhLoRa(PA4,PA12);//NSS, DIO0
KeyFob device(2, &cfg, &rhLoRa, &rhSerial, PA11, PB1);
#endif
}
#endif

#define DEBUG

void setup()
{
#ifdef DEBUG
    Serial.begin(9600);
    //Serial port will only be connected in debug mode
    while (!Serial) ; // Wait for serial port to be available
#endif
    ld=&device;
    if(ld->setup() && ld->init())
    {
#ifdef DEBUG
        Serial.println("Init Ok");
#endif
    }
    else
    {
#ifdef DEBUG
        Serial.println("Init false");
#endif
    }
    if(cfg.init())
    {
#ifdef DEBUG
        Serial.println("Config valid");
#endif
    }
}

void loop()
{
    ld->loop();
}

void print(const byte* array, byte length)
{
    Serial.print("Length = ");Serial.println(length,DEC);
    for (byte i = 0; i < length; i++)
    {
        Serial.print(array[i], HEX);
        Serial.print(" ");
        if ((i + 1) % 16 == 0)
        {
            Serial.println();
        }
    }
    Serial.println();
}



