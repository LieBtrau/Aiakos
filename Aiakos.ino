/* Connections to Nucleo F103RB
* XL1276    Protrinket  Due    Nucleo   ATSHA204A
*           3V
* ---------------------------------------------------
* NSS       10          4       A2
* MOSI      11          ICSP.4  D11
* MISO      12          ICSP.1  D12
* SCK       13          ICSP.3  D13  (Be careful, this is the 6th pin on the left row of the right most pin header connector, not the fifth!)
* REST      RST         RESET   NRST
* DIO0      3           3       D5
* VCC       3V          3.3V    3V
* GND       G           GND     GND
*                       RX1     D8
*                       TX1     D2
*                               D3      SDA
*                               D4      SCL
*/

#include <RHReliableDatagram.h> //for wireless comm
#include <RH_RF95.h>            //for wireless comm
#include <RH_Serial.h>          //for wired comm
#include <SPI.h>                //for wireless comm
#include "kryptoknight.h"       //for authentication
#include "cryptoauthlib.h"      //for TRNG & serial number
#include "ecdhcomm.h"           //for secure pairing
#include "configuration.h"      //for non-volatile storage of parameters

#define DEBUG

const byte ADDRESS1=1;
const byte ADDRESS2=2;
RH_Serial rhSerial(Serial1);
byte id2[]={9,8,7,6,5,4,3,2,1};
Configuration cfg;

#ifdef ARDUINO_STM_NUCLEO_F103RB

//STM Nucleo = Garage controller
byte payload[4]={0xFE, 0xDC, 0xBA, 0x98};
Kryptoknight k1= Kryptoknight(&ATSHA_RNG, writeDataLoRa, readDataLoRa);
EcdhComm ecdh1= EcdhComm(&ATSHA_RNG, writeDataSer, readDataSer);
RH_RF95 rhLoRa(A2,5);//NSS, DIO0
RHReliableDatagram mgrLoRa(rhLoRa, ADDRESS1);
RHReliableDatagram mgrSer(rhSerial, ADDRESS1);
ATCAIfaceCfg *gCfg = &cfg_sha204a_i2c_default;

#elif defined(ARDUINO_AVR_PROTRINKET3)
#error Target no longer supported because of lack of RAM space
//Adafruit ProTrinket = Key fob
Kryptoknight k2= Kryptoknight(id2,IDLENGTH,sharedkey, &RNG, writeData, readData);
RH_RF95 driver(10,3);
RHReliableDatagram manager(driver, ADDRESS2);

#elif defined(ARDUINO_SAM_DUE)
//Arduino Due = Key fob
Kryptoknight k2= Kryptoknight(id2, Configuration::IDLENGTH, &RNG, writeDataLoRa, readDataLoRa);
EcdhComm ecdh2=EcdhComm(&RNG, writeDataSer, readDataSer);
RH_RF95 rhLoRa(4,3);
RHReliableDatagram mgrLoRa(rhLoRa, ADDRESS2);
RHReliableDatagram mgrSer(rhSerial, ADDRESS2);

#else

#error No device type defined.

#endif

void setup()
{
    Serial.begin(9600);
    rhSerial.serial().begin(9600);
    while (!Serial) ; // Wait for serial port to be available
    if ((!mgrLoRa.init()))
    {
#ifdef DEBUG
        Serial.println("LoRa init failed");
#endif
        return;
    }
    if ((!mgrSer.init()))
    {
#ifdef DEBUG
        Serial.println("Serial init failed");
#endif
        return;
    }
#ifdef ARDUINO_STM_NUCLEO_F103RB
    byte buf[10];
    if((!getSerialNumber(buf, Configuration::IDLENGTH)) || (!k1.setLocalId(buf,Configuration::IDLENGTH) ) || (!ecdh1.init(buf, Configuration::IDLENGTH)) )
    {
        return;
    }
    if(!cfg.init())
    {
#ifdef DEBUG
        Serial.println("Config invalid");
#endif
        if(!ecdh1.startPairing())
        {
            Serial.println("Sending message failed.");
            return;
        }
    }
    else
    {
#ifdef DEBUG
        Serial.println("Config valid");
#endif
        k1.setSharedKey(cfg.getKey(0));
        Serial.println("Initiator starts authentication");
        if(!k1.sendMessage(id2,payload,4))
        {
            Serial.println("Sending message failed.");
            return;
        }
    }
#elif defined(ARDUINO_AVR_PROTRINKET3) || defined(ARDUINO_SAM_DUE)
    if(!cfg.init())
    {
#ifdef DEBUG
        Serial.println("Config invalid");
#endif
    }
    else
    {
#ifdef DEBUG
        Serial.println("Config valid");
#endif
        k2.setSharedKey(cfg.getKey(0));
    }
    if(!ecdh2.init(id2, Configuration::IDLENGTH))
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
    if(k1.loop()==Kryptoknight::AUTHENTICATION_AS_INITIATOR_OK)
    {
        Serial.println("Message received by peer and acknowledged");
    }
    if(ecdh1.loop()==EcdhComm::AUTHENTICATION_OK)
    {
        Serial.println("Securely paired");
        k1.setSharedKey(ecdh1.getMasterKey());
        saveKey();
    }
#elif defined(ARDUINO_AVR_PROTRINKET3) || defined(ARDUINO_SAM_DUE)
    if(k2.loop()==Kryptoknight::AUTHENTICATION_AS_PEER_OK)
    {
        Serial.println("Message received by remote initiator");
    }
    if(ecdh2.loop()==EcdhComm::AUTHENTICATION_OK)
    {
        Serial.println("Securely paired");
        k2.setSharedKey(ecdh2.getMasterKey());
        saveKey();
    }
#else
#error No device
#endif
}

bool writeDataSer(byte* data, byte length)
{
#ifdef DEBUG
    Serial.println("Sending data: ");print(data, length);
#endif
#ifdef ARDUINO_STM_NUCLEO_F103RB
    return mgrSer.sendtoWait(data, length, ADDRESS2);
#elif defined(ARDUINO_AVR_PROTRINKET3) || defined(ARDUINO_SAM_DUE)
    return mgrSer.sendtoWait(data, length, ADDRESS1);
#else
#error No device
#endif
}

bool writeDataLoRa(byte* data, byte length)
{
#ifdef DEBUG
    Serial.println("Sending data: ");print(data, length);
#endif
#ifdef ARDUINO_STM_NUCLEO_F103RB
    return mgrLoRa.sendtoWait(data, length, ADDRESS2);
#elif defined(ARDUINO_AVR_PROTRINKET3) || defined(ARDUINO_SAM_DUE)
    return mgrLoRa.sendtoWait(data, length, ADDRESS1);
#else
#error No device
#endif
}

bool readDataSer(byte** data, byte& length)
{
    byte from;
    if (!mgrSer.available())
    {
        return false;
    }
    if(!mgrSer.recvfromAck(*data, &length, &from))
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

bool readDataLoRa(byte** data, byte& length)
{
    byte from;
    if (!mgrLoRa.available())
    {
        return false;
    }
    if(!mgrLoRa.recvfromAck(*data, &length, &from))
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

void saveKey()
{
#ifdef ARDUINO_STM_NUCLEO_F103RB
    cfg.setKey(0,ecdh1.getRemoteId(), ecdh1.getMasterKey());
#elif defined(ARDUINO_AVR_PROTRINKET3) || defined(ARDUINO_SAM_DUE)
    cfg.setKey(0,ecdh2.getRemoteId(), ecdh2.getMasterKey());
#endif
    cfg.saveData();
}

void dataReceived(byte* data, byte length)
{
    Serial.println("Event received with the following data:");
    print(data, length);
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
