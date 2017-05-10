/* Hardware Connections
 * **************************************************
 * LoRa module
 * **************************************************
 * XL1276    Protrinket  Due    Nucleo   ATSHA204A
 *           3V
 *****************************************************
 * NSS       10          4       A2
 * MOSI      11          ICSP.4  D11
 * MISO      12          ICSP.1  D12
 * SCK       13          ICSP.3  D13  (Be careful, this is the 6th pin on the left row of the right most pin header connector, not the fifth!)
 * REST      RST         RESET   NRST
 * DIO0      3           3       D5
 * VCC       3V          3.3V    3V
 * GND       G           GND     GND
 *
 *****************************************************
 * ATSHA204A for TRNG and unique serial number
 *****************************************************
 *                               D3      5 (SDA)
 *                               D4      6 (SCL)
 *
 *****************************************************
 * Serial connection for secure pairing
 *****************************************************
 * Tip                   TX1     D2
 * Ring                  RX1     D8
 * Sleeve                GND     GND
 * Cable detect Contact  2       D6
 * Pushbutton                    25
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

#include "loradevice.h"
#include "keyfob.h"
#include "garagecontroller.h"

#ifdef ARDUINO_STM_NUCLEO_F103RB
#define ROLE_KEYFOB
#elif defined(ARDUINO_SAM_DUE)
#define ROLE_GARAGE_CONTROLLER
#else
#error No device type defined.
#endif

namespace
{
Configuration cfg;
RH_Serial rhSerial(Serial1);
#ifdef ROLE_GARAGE_CONTROLLER
    #include "garagecontroller.h"
    RH_RF95 rhLoRa(4,3);
    GarageController device(1,2, &cfg, &rhLoRa, &rhSerial);
#elif defined(ROLE_KEYFOB)
    #include "keyfob.h"
    RH_RF95 rhLoRa(A2,5);//NSS, DIO0
    KeyFob device(2,1, &cfg, &rhLoRa, &rhSerial);
#endif
LoRaDevice* ld;
#define DEBUG
}

void setup()
{
    ld=&device;
    ld->setup();
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



