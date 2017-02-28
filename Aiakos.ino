#include <RHReliableDatagram.h> //for wireless comm
#include <RH_RF95.h>            //for wireless comm
#include <SPI.h>
#include "kryptoknight.h"       //for authentication
#include "cryptoauthlib.h"      //for TRNG & serial number
#include "ecdhcomm.h"           //for secure pairing

#define DEBUG

void dataReceived(byte* data, byte length)
{
    Serial.println("Event received with the following data:");
    print(data, length);
}

const byte IDLENGTH=9;
byte sharedkey[16]={0xA,0xB,0xC,0xD,
                    0xE,0xF,0xE,0xD,
                    0xC,0xB,0xA,0x9,
                    0x8,0x7,0x6,0x5};
byte id2[]={9,8,7,6,5,4,3,2,1};

/* Connections to Nucleo F103RB
* XL1276   Protrinket 3V    Due    Nucleo
* ---------------------------------------
* NSS      10               4       A2
* MOSI     11               ICSP.4  D11
* MISO     12               ICSP.1  D12
* SCK      13               ICSP.3  D13  (Be careful, this is the 6th pin on the left row of the right most pin header connector, not the fifth!)
* REST     RST              RESET   NRST
* DIO0     3                3       D2
* VCC      3V               3.3V    3V
* GND      G                GND     GND
*
*/
const byte ADDRESS1=1;
const byte ADDRESS2=2;

#ifdef ARDUINO_STM_NUCLEO_F103RB
//STM Nucleo = Garage controller
byte payload[4]={0xFE, 0xDC, 0xBA, 0x98};
Kryptoknight k1= Kryptoknight(sharedkey,&ATSHA_RNG, writeData, readData);
EcdhComm ecdh1= EcdhComm(&ATSHA_RNG, writeData, readData);
RH_RF95 driver(A2,2);//NSS, DIO0
RHReliableDatagram manager(driver, ADDRESS1);
ATCAIfaceCfg *gCfg = &cfg_sha204a_i2c_default;
#elif defined(ARDUINO_AVR_PROTRINKET3)
#error Target no longer supported because of lack of RAM space
////Adafruit ProTrinket = Key fob
//Kryptoknight k2= Kryptoknight(id2,IDLENGTH,sharedkey, &RNG, writeData, readData);
//RH_RF95 driver(10,3);
//RHReliableDatagram manager(driver, ADDRESS2);
#elif defined(ARDUINO_SAM_DUE)
//Arduino Due = Key fob
Kryptoknight k2= Kryptoknight(id2,IDLENGTH,sharedkey, &RNG, writeData, readData);
EcdhComm ecdh2=EcdhComm(&RNG, writeData, readData);
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
        return;
    }
#ifdef ARDUINO_STM_NUCLEO_F103RB
    byte buf[10];
    if((!getSerialNumber(buf,IDLENGTH)) || (!k1.setLocalId(buf,IDLENGTH) ) || (!ecdh1.init(buf, IDLENGTH)) )
    {
       return;
    }
    if(!ecdh1.startPairing())
    {
        Serial.println("Sending message failed.");
    }
//    Serial.println("Initiator starts authentication");
//    if(!k1.sendMessage(id2,payload,4))
//    {
//        Serial.println("Sending message failed.");
//        return;
//    }
#elif defined(ARDUINO_AVR_PROTRINKET3) || defined(ARDUINO_SAM_DUE)
    if(!ecdh2.init(id2, IDLENGTH))
    {
        return;
    }
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
//    if(k1.loop()==Kryptoknight::AUTHENTICATION_AS_INITIATOR_OK)
//    {
//        Serial.println("Message received by peer and acknowledged");
//    }
    if(ecdh1.loop()==EcdhComm::AUTHENTICATION_OK)
    {
        Serial.println("Message received by peer and acknowledged");
    }
#elif defined(ARDUINO_AVR_PROTRINKET3) || defined(ARDUINO_SAM_DUE)
//    if(k2.loop()==Kryptoknight::AUTHENTICATION_AS_PEER_OK)
//    {
//        Serial.println("Message received by remote initiator");
//    }
    if(ecdh2.loop()==EcdhComm::AUTHENTICATION_OK)
    {
        Serial.println("Message received by peer and acknowledged");
    }
#else
//#error No device
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

//Dummy function to read incoming data on device 1
bool readData(byte** data, byte& length)
{
    byte from;
    if (!manager.available())
    {
        return false;
    }
    if(!manager.recvfromAck(*data, &length, &from))
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
    return true;
}

#ifdef ARDUINO_STM_NUCLEO_F103RB
static int ATSHA_RNG(byte *dest, unsigned size)
{
    byte randomnum[RANDOM_RSP_SIZE];

    if(atcab_init( gCfg ) != ATCA_SUCCESS)
    {
        return 0;
    }
    while(size)
    {
        if(atcab_random( randomnum ) != ATCA_SUCCESS)
        {
            return 0;
        }
        byte nrOfBytes = size > 32 ? 32 : size;
        memcpy(dest,randomnum, nrOfBytes);
        dest+=nrOfBytes;
        size-=nrOfBytes;
    }
    if(atcab_release() != ATCA_SUCCESS)
    {
        return 0;
    }
    return 1;
}
bool getSerialNumber(byte* bufout, byte length)
{
    byte buf[11];
    if(atcab_init( gCfg ) != ATCA_SUCCESS)
    {
        return false;
    }
    if(atcab_read_serial_number(buf) != ATCA_SUCCESS)
    {
        return false;
    }
    if(atcab_release() != ATCA_SUCCESS)
    {
        return false;
    }
    memcpy(bufout, buf, length > 9 ? 9 : length);
    return true;
}

#endif

//TODO: replace by safe external RNG
static int RNG(uint8_t *dest, unsigned size) {
    // Use the least-significant bits from the ADC for an unconnected pin (or connected to a source of
    // random noise). This can take a long time to generate random data if the result of analogRead(0)
    // doesn't change very frequently.
#ifdef ARDUINO_AVR_PROTRINKET3
        byte adcpin=0;
#elif defined(ARDUINO_SAM_DUE) || defined(ARDUINO_STM_NUCLEO_F103RB)
        byte adcpin=A0;
#endif
    while (size) {
    uint8_t val = 0;
    for (unsigned i = 0; i < 8; ++i) {
        int init = analogRead(adcpin);
        int count = 0;
        while (analogRead(adcpin) == init) {
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
