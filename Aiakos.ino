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
#include "kryptoknightcomm.h"   //for authentication
#include "ecdhcomm.h"           //for secure pairing
#include "configuration.h"      //for non-volatile storage of parameters
#include "cryptohelper.h"

#define DEBUG

const byte ADDRESS1=1;
const byte ADDRESS2=2;

bool writeDataLoRa(byte* data, byte length);
bool readDataLoRa(byte** data, byte& length);
bool readDataSer(byte** data, byte& length);
bool writeDataSer(byte* data, byte length);

RH_Serial rhSerial(Serial1);
Configuration cfg;
KryptoKnightComm k= KryptoKnightComm(&RNG, writeDataLoRa, readDataLoRa);
EcdhComm ecdh= EcdhComm(&RNG, writeDataSer, readDataSer);

#ifdef ARDUINO_STM_NUCLEO_F103RB
#define ROLE_KEYFOB
#elif defined(ARDUINO_SAM_DUE)
#define ROLE_GARAGE_CONTROLLER
#else
#error No device type defined.
#endif

#ifdef ARDUINO_STM_NUCLEO_F103RB
RH_RF95 rhLoRa(A2,5);//NSS, DIO0
#elif defined(ARDUINO_SAM_DUE)
RH_RF95 rhLoRa(4,3);
#endif

#ifdef ROLE_GARAGE_CONTROLLER
RHReliableDatagram mgrLoRa(rhLoRa, ADDRESS1);
RHReliableDatagram mgrSer(rhSerial, ADDRESS1);
#elif defined(ROLE_KEYFOB)
byte payload[4]={0xFE, 0xDC, 0xBA, 0x98};
RHReliableDatagram mgrLoRa(rhLoRa, ADDRESS2);
RHReliableDatagram mgrSer(rhSerial, ADDRESS2);
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
            || (!k.init(buf,Configuration::IDLENGTH) )
            || (!ecdh.init(buf, Configuration::IDLENGTH)) )
    {
        return;
    }
    if(cfg.init())
    {
#ifdef DEBUG
        Serial.println("Config valid");
#endif
#ifdef ROLE_KEYFOB
        Serial.println("Initiator starts authentication");
        if(!k.sendMessage(payload,sizeof(payload), cfg.getDefaultId(), cfg.getIdLength(), cfg.getDefaultKey()))
        {
            Serial.println("Sending message failed.");
            return;
        }
#elif defined(ROLE_GARAGE_CONTROLLER)
        k.setMessageReceivedHandler(dataReceived);
        k.setKeyRequestHandler(setKeyInfo);
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
        cfg.addKey(ecdh.getRemoteId(), ecdh.getMasterKey());
    }
#ifdef ROLE_KEYFOB
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
#elif defined(ROLE_GARAGE_CONTROLLER)
    if(k.loop()==KryptoKnightComm::AUTHENTICATION_AS_PEER_OK)
    {
        Serial.println("Message received by remote initiator");
    }
#endif
}

bool writeDataSer(byte* data, byte length)
{
    Serial.print("Sending data...");
#ifdef ROLE_GARAGE_CONTROLLER
#ifdef DEBUG
    Serial.println("to key fob: ");print(data, length);
#endif
    return mgrSer.sendtoWait(data, length, ADDRESS2);
#elif defined(ROLE_KEYFOB)
#ifdef DEBUG
    Serial.println("to garage controller: ");print(data, length);
#endif
    return mgrSer.sendtoWait(data, length, ADDRESS1);
#endif
}

bool writeDataLoRa(byte* data, byte length)
{
#ifdef DEBUG
    Serial.println("Sending data: ");print(data, length);
#endif
#ifdef ROLE_GARAGE_CONTROLLER
    return mgrLoRa.sendtoWait(data, length, ADDRESS2);
#elif defined(ROLE_KEYFOB)
    return mgrLoRa.sendtoWait(data, length, ADDRESS1);
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
#ifdef ROLE_GARAGE_CONTROLLER
    if(from != ADDRESS2)
    {
#ifdef DEBUG
        Serial.println("Sender doesn't match");
#endif
        return false;
    }
#elif defined(ROLE_KEYFOB)
    if(from != ADDRESS1)
    {
#ifdef DEBUG
        Serial.println("Sender doesn't match");
#endif
        return false;
    }
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
#ifdef ROLE_GARAGE_CONTROLLER
    if(from != ADDRESS2)
    {
#ifdef DEBUG
        Serial.println("Sender doesn't match");
#endif
        return false;
    }
#elif defined(ROLE_KEYFOB)
    if(from != ADDRESS1)
    {
#ifdef DEBUG
        Serial.println("Sender doesn't match");
#endif
        return false;
    }
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
    Serial.println("Remote ID Event received with the following data:");
    print(remoteId, length);
    byte* key = cfg.findKey(remoteId, length);
    if(key)
    {
        k.setRemoteParty(remoteId, length, key);
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
