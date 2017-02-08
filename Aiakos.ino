#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>
#include "kryptoknight.h"

#define DEBUG

static int RNG(uint8_t *dest, unsigned size);
void print(const byte* array, byte length);

void dataReceived(byte* data, byte length)
{
    Serial.println("Event received with the following data:");
    print(data, length);
}

const byte IDLENGTH=10;
byte sharedkey[16]={0xA,0xB,0xC,0xD,
                    0xE,0xF,0xE,0xD,
                    0xC,0xB,0xA,0x9,
                    0x8,0x7,0x6,0x5};
byte id2[IDLENGTH]={9,8,7,6,5,4,3,2,1,0};

/* Connections to Nucleo F103RB
* XL1276   Protrinket 3V    Nucleo
* --------------------------------
* NSS      10               A2
* MOSI     11               D11
* MISO     12               D12
* SCK      13               D13  (Be careful, this is the 6th pin on the left row of the right most pin header connector, not the fifth!)
* REST     RST              NRST
* DIO0     3                D2
* VCC      3V               3V
* GND      G                GND
*
*/
const byte ADDRESS1=1;
const byte ADDRESS2=2;

#ifdef ARDUINO_STM_NUCLEO_F103RB
//STM Nucleo
byte payload[4]={0xFE, 0xDC, 0xBA, 0x98};
byte id1[IDLENGTH]={0,1,2,3,4,5,6,7,8,9};
Kryptoknight k1= Kryptoknight(id1,IDLENGTH,sharedkey, &RNG, writeData, readData);
RH_RF95 driver(A2,2);//NSS, DIO0
RHReliableDatagram manager(driver, ADDRESS1);
#elif defined(ARDUINO_AVR_PROTRINKET3)
//Adafruit ProTrinket
Kryptoknight k2= Kryptoknight(id2,IDLENGTH,sharedkey, &RNG, writeData, readData);
RH_RF95 driver(10,3);
RHReliableDatagram manager(driver, ADDRESS2);
#elif defined(ARDUINO_SAM_DUE)
//Arduino Due
Kryptoknight k2= Kryptoknight(id2,IDLENGTH,sharedkey, &RNG, writeData, readData);
RH_RF95 driver(4,3);
RHReliableDatagram manager(driver, ADDRESS2);
#else
#error No device type defined.
#endif

void setup()
{
    Serial.begin(9600);
    while (!Serial) ; // Wait for serial port to be available
    if (!manager.init())
    {
#ifdef DEBUG
        Serial.println("init failed");
#endif
    }
#ifdef ARDUINO_STM_NUCLEO_F103RB
    Serial.println("Initiator starts authentication");
    if(!k1.sendMessage(id2,payload,4))
    {
        Serial.println("Sending message failed.");
        return;
    }
#elif defined(ARDUINO_AVR_PROTRINKET3) || defined(ARDUINO_SAM_DUE)
    k2.setMessageReceivedHandler(dataReceived);
#else
#error No device
#endif
#ifdef DEBUG
        Serial.println("ready");
#endif

}


void loop()
{
#ifdef ARDUINO_STM_NUCLEO_F103RB
    if(k1.loop()==Kryptoknight::AUTHENTICATION_AS_INITIATOR_OK)
    {
        Serial.println("Message received by peer and acknowledged");
    }
#elif defined(ARDUINO_AVR_PROTRINKET3) || defined(ARDUINO_SAM_DUE)
    if(k2.loop()==Kryptoknight::AUTHENTICATION_AS_PEER_OK)
    {
        Serial.println("Message received by remote initiator");
    }
#else
#error No device
#endif
}

//Dummy function to write data from device 2 to device 1
bool writeData(byte* data, byte length)
{
#ifdef DEBUG
        Serial.println("Sending data: ");print(data, length);
#endif
#ifdef ARDUINO_STM_NUCLEO_F103RB
    return manager.sendtoWait(data, length, ADDRESS2);
#elif defined(ARDUINO_AVR_PROTRINKET3) || defined(ARDUINO_SAM_DUE)
    return manager.sendtoWait(data, length, ADDRESS1);
#else
#error No device
#endif
}

// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

//Dummy function to read incoming data on device 1
bool readData(byte** data, byte& length)
{
    byte from;
    if (!manager.available())
    {
        return false;
    }
    if(!manager.recvfromAck(buf, &length, &from))
    {
        return false;
    }
#ifdef ARDUINO_STM_NUCLEO_F103RB
    if(from != ADDRESS2)
    {
#ifdef DEBUG
        Serial.println("Sender doesn't match");
#endif
        return false;
    }
#elif defined(ARDUINO_AVR_PROTRINKET3) || defined(ARDUINO_SAM_DUE)
    if(from != ADDRESS1)
    {
#ifdef DEBUG
        Serial.println("Sender doesn't match");
#endif
        return false;
    }
#else
#error No device
#endif
    *data=buf;
    return true;
}

//TODO: replace by safe external RNG
static int RNG(uint8_t *dest, unsigned size) {
    // Use the least-significant bits from the ADC for an unconnected pin (or connected to a source of
    // random noise). This can take a long time to generate random data if the result of analogRead(0)
    // doesn't change very frequently.
    while (size) {
    uint8_t val = 0;
    for (unsigned i = 0; i < 8; ++i) {
        int init = analogRead(0);
        int count = 0;
        while (analogRead(0) == init) {
        ++count;
        }

        if (count == 0) {
        val = (val << 1) | (init & 0x01);
        } else {
        val = (val << 1) | (count & 0x01);
        }
    }
    *dest = val;
    ++dest;
    --size;
    }
    // NOTE: it would be a good idea to hash the resulting random data using SHA-256 or similar.
    return 1;
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
