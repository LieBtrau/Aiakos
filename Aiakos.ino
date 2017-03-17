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
#include "kryptoknightcomm.h"  //for authentication
#include "ecdhcomm.h"           //for secure pairing
#include "configuration.h"      //for non-volatile storage of parameters
#include "cryptohelper.h"

#define DEBUG

const byte ADDRESS1=1;
const byte ADDRESS2=2;
RH_Serial rhSerial(Serial1);
Configuration cfg;

bool writeDataLoRa(byte* data, byte length);
bool readDataLoRa(byte** data, byte& length);
bool readDataSer(byte** data, byte& length);
bool writeDataSer(byte* data, byte length);


#ifdef ARDUINO_STM_NUCLEO_F103RB

//STM Nucleo = Garage controller
byte payload[4]={0xFE, 0xDC, 0xBA, 0x98};
KryptoKnightComm k= KryptoKnightComm(&RNG, writeDataLoRa, readDataLoRa);
EcdhComm ecdh= EcdhComm(&RNG, writeDataSer, readDataSer);
RH_RF95 rhLoRa(A2,5);//NSS, DIO0
RHReliableDatagram mgrLoRa(rhLoRa, ADDRESS1);
RHReliableDatagram mgrSer(rhSerial, ADDRESS1);

#elif defined(ARDUINO_SAM_DUE)
//Arduino Due = Key fob
KryptoKnightComm k= KryptoKnightComm(&RNG, writeDataLoRa, readDataLoRa);
EcdhComm ecdh=EcdhComm(&RNG, writeDataSer, readDataSer);
RH_RF95 rhLoRa(4,3);
RHReliableDatagram mgrLoRa(rhLoRa, ADDRESS2);
RHReliableDatagram mgrSer(rhSerial, ADDRESS2);

#else

#error No device type defined.

#endif

void setup()
{
    Serial.begin(9600);
    rhSerial.serial().begin(2400);
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
#ifdef ARDUINO_SAM_DUE
    initRng();
#endif
    byte buf[10];
    if((!getSerialNumber(buf, Configuration::IDLENGTH))
            || (!k.init(buf,Configuration::IDLENGTH) ) || (!ecdh.init(buf, Configuration::IDLENGTH)) )
    {
        return;
    }
    if(cfg.init())
    {
#ifdef DEBUG
        Serial.println("Config valid");
#endif
#ifdef ARDUINO_STM_NUCLEO_F103RB
        Serial.println("Initiator starts authentication");
        if(!k.sendMessage(payload,sizeof(payload), cfg.getId(0), cfg.getIdLength(), cfg.getKey(0)))
        {
            Serial.println("Sending message failed.");
            return;
        }
#elif defined(ARDUINO_SAM_DUE)
        k.setMessageReceivedHandler(dataReceived);
        k.setKeyRequestHandler(setKeyInfo);
#else
#error No device
#endif
    }

#ifdef DEBUG
    Serial.println("ready");
#endif

}


void loop()
{
    if(ecdh.loop()==EcdhComm::AUTHENTICATION_OK)
    {
#ifdef DEBUG
        Serial.println("Securely paired");
#endif
        cfg.setKey(0,ecdh.getRemoteId(), ecdh.getMasterKey());
        cfg.saveData();
    }
#ifdef ARDUINO_STM_NUCLEO_F103RB
    if(k.loop()==KryptoKnightComm::AUTHENTICATION_AS_INITIATOR_OK)
    {
        Serial.println("Message received by peer and acknowledged");
    }
    if(!digitalRead(25))
    {
        if(!ecdh.startPairing())
        {
            Serial.println("Sending message failed.");
            return;
        }
    }
#elif defined(ARDUINO_SAM_DUE)
    if(k.loop()==KryptoKnightComm::AUTHENTICATION_AS_PEER_OK)
    {
        Serial.println("Message received by remote initiator");
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
#ifdef DEBUG
    Serial.println("Received data: ");print(*data, length);
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

void dataReceived(byte* data, byte length)
{
    Serial.println("Event received with the following data:");
    print(data, length);
}

void setKeyInfo(byte* remoteId, byte length)
{
    Serial.println("ID Event received with the following data:");
    print(remoteId, length);
    byte keyIndex=cfg.findKeyIndex(remoteId, length);
    if( keyIndex != 255)
    {
        k.setRemoteParty(cfg.getId(keyIndex), cfg.getIdLength(), cfg.getKey(keyIndex));
    }
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
