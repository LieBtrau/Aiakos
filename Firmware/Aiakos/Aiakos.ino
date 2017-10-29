
#include "loradevice.h"
#include "debug.h"
#ifndef ARDUINO_SAM_DUE
#include "STM32Sleep.h"
#include <RTClock.h>
#endif

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
 * DIO0      3           3       D5     PB0
 * VCC       3V          3.3V    3V     3.3
 * GND       G           GND     GND    G
 *
 *****************************************************
 * ATSHA204A for TRNG and unique serial number (bitbanged-I²C, because Arduino Hardwire I²C buffers are too small)
 *****************************************************
                Blue Pill   Nucleo   ATSHA204A
 *****************************************************
 *              PB9         D3      5 (SDA)
 *              PB8         D4      6 (SCL)
 *
 *****************************************************
 * Serial connection for secure pairing
 *                      Due     Nucleo(Keyfob)  Blue Pill
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
 *****************************************************
 * Tone output on key fob
 *****************************************************
 * Tone                  A1
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
unsigned long ledTimer;
}

#ifdef ROLE_GARAGE_CONTROLLER

#include "garagecontroller.h"
namespace
{
RH_RF95 rhLoRa(4,3);
RH_Serial rhStereoJack(Serial1);
GarageController device(1, &cfg, &rhLoRa, &rhStereoJack, 2);
const byte pinLED=13;
}

#elif defined(ROLE_KEYFOB)

#include "keyfob.h"
#include "blecontrol.h"
namespace
{
#ifdef ARDUINO_STM_NUCLEO_F103RB
RH_Serial rhStereoJack(Serial1);
RH_RF95 rhLoRa(A2,5);//NSS, DIO0
KeyFob device(2, &cfg, &rhLoRa, &rhStereoJack, 25, 6, &ble);
#elif defined(ARDUINO_GENERIC_STM32F103C)                           //Blue Pill
RH_Serial rhStereoJack(Serial2);                                    //UART3: Serial port for pairing
RH_RF95 rhLoRa(PA4,PB0);                                           //NSS, DIO0 : for long range wireless
/*Connections to BLE RN4020 module
 * |    Blue pill       |   RN4020
 * -----------------------------------------
 * | 3V                 |   23 (VDD)
 * | G                  |   24 (GND)
 * | PA3 (UART2.RX)     |   5 (UART_TX)
 * | PA2 (UART2.TX)     |   6 (UART_RX)
 * | PB12               |   7 (WAKE_SW)
 * | PB15               |   12 (ACTIVE)
 * | PB14               |   15 (WAKE_HW)
 * | PB13               |   /  EN_PWR
 * | PA15               |   10 (CONNECTION_LED)
 */
rn4020 rn(Serial1, PB12, PB15, PB14, PB13, PA15);
bleControl ble(&rn);
KeyFob device(2, &cfg, &rhLoRa, &rhStereoJack, PA11, PB1, &ble, PA1);
RTClock rt(RTCSEL_LSE);
const byte pinLED=PC13; //Active low
#endif
}

#endif

void setup()
{
    ld=&device;
    ld->getInitialPinStates();
//    pinMode(PA8, OUTPUT);
//    digitalWrite(PA8, HIGH);//It takes 463ms to get here
//    delay(100);
    pinMode(pinLED, OUTPUT);
    ledTimer=millis();
    openDebug(9600);
    debug_println("Waking up");
    if(!cfg.init())
    {
        debug_println("Config invalid");
        while(1);
    }
    if(!ld->setup())
    {
        debug_println("Setup failed");
//        while(1);
    }
}

void loop()
{
    ld->loop();
#ifdef DEBUG
    ledToggle(500);
}

void ledToggle(unsigned long timeOut)
{
    if(millis()>ledTimer+timeOut)
    {
        digitalWrite(pinLED, digitalRead(pinLED) ? 0 : 1);
        ledTimer=millis();
    }
#endif
}
