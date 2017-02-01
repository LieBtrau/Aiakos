#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <RH_Serial.h>
#include <RH_SwSerial.h>
#include <SPI.h>
#include "keyagreement.h"

/* Connections to Nucleo F103RB
* XL1276   Protrinket 3V    Nucleo
* --------------------------------
* NSS      10               A2
* MOSI     11               D11
* MISO     12               D12
* SCK      13               D13  (Be careful, this is the 6th pin on the left row of the right most pin header connector, not the fifth!)
* REST     RST              NRST
* DIO0     3                D3
* VCC      3V               3V
* GND      G                GND
*
*/

const byte CLIENT_ADDRESS=1;
const byte SERVER_ADDRESS=2;

#ifdef ARDUINO_STM_NUCLEO_F103RB
//Nucleo
uint8_t data[] = "And hello back to you";
RH_RF95 driver(A2,3);
RHReliableDatagram manager(driver, SERVER_ADDRESS);
RH_Serial driverSerial(Serial1);
RHReliableDatagram managerSerial(driverSerial, SERVER_ADDRESS);
Keyagreement keyagr(true);

#elif defined(ARDUINO_AVR_PROTRINKET3)
//ProTrinket
uint8_t data[] = "Hello World!";
RH_RF95 driver(10,3);
RHReliableDatagram manager(driver, CLIENT_ADDRESS);
#include <SoftwareSerial.h>
SoftwareSerial SerialSw(4, 5); // RX, TX
RH_SwSerial driverSerial(SerialSw);
RHReliableDatagram managerSerial(driverSerial, CLIENT_ADDRESS);
Keyagreement keyagr(false);
#else
#error No device type defined.
#endif

void setup()
{
    Serial.begin(9600);
    while (!Serial) ; // Wait for serial port to be available
    Serial.println("Hallo!");
    if (!manager.init())
    {
        Serial.println("init failed");
    }
    driverSerial.serial().begin(9600);
    if (!managerSerial.init())
    {
        Serial.println("init serial failed");
    }
#ifdef ARDUINO_STM_NUCLEO_F103RB
    if(!keyagr.runKeyAgreement(managerSerial, CLIENT_ADDRESS))
    {
        Serial.println("key agreement failed");
    }
#elif defined(ARDUINO_AVR_PROTRINKET3)
    if(!keyagr.runKeyAgreement(managerSerial, SERVER_ADDRESS))
    {
        Serial.println("key agreement failed");
    }
#endif
}

void loop()
{

//#ifdef ARDUINO_STM_NUCLEO_F103RB
//    if (managerSerial.available())
//    {
//        // Wait for a message addressed to us from the client
//        uint8_t len = sizeof(buf);
//        uint8_t from;
//        if (managerSerial.recvfromAck(buf, &len, &from))
//        {
//            Serial.print("got request from : 0x");
//            Serial.print(from, HEX);
//            Serial.print(": ");
//            Serial.println((char*)buf);

//            // Send a reply back to the originator client
//            if (!managerSerial.sendtoWait(data, sizeof(data), from))
//                Serial.println("sendtoWait failed");
//        }
//    }
//#elif defined(ARDUINO_AVR_PROTRINKET3)
//    Serial.println("Sending to rf95_reliable_datagram_server");

//    // Send a message to manager_server
//    if (managerSerial.sendtoWait(data, sizeof(data), SERVER_ADDRESS))
//    {
//        // Now wait for a reply from the server
//        uint8_t len = sizeof(buf);
//        uint8_t from;
//        if (managerSerial.recvfromAckTimeout(buf, &len, 2000, &from))
//        {
//            Serial.print("got reply from : 0x");
//            Serial.print(from, HEX);
//            Serial.print(": ");
//            Serial.println((char*)buf);
//        }
//        else
//        {
//            Serial.println("No reply, is rf95_reliable_datagram_server running?");
//        }
//    }
//    else
//        Serial.println("sendtoWait failed");
//    delay(500);
//#endif
}
