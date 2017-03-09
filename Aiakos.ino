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
Kryptoknight k= Kryptoknight(&ATSHA_RNG, writeDataLoRa, readDataLoRa);
EcdhComm ecdh= EcdhComm(&ATSHA_RNG, writeDataSer, readDataSer);
RH_RF95 rhLoRa(A2,5);//NSS, DIO0
RHReliableDatagram mgrLoRa(rhLoRa, ADDRESS1);
RHReliableDatagram mgrSer(rhSerial, ADDRESS1);

#elif defined(ARDUINO_AVR_PROTRINKET3)
#error Target no longer supported because of lack of RAM space
//Adafruit ProTrinket = Key fob
Kryptoknight k= Kryptoknight(id2,IDLENGTH,sharedkey, &RNG, writeData, readData);
RH_RF95 rhLoRa(10,3);
RHReliableDatagram manager(rhLoRa, ADDRESS2);

#elif defined(ARDUINO_SAM_DUE)
//Arduino Due = Key fob
Kryptoknight k= Kryptoknight(&RNG, writeDataLoRa, readDataLoRa);
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
        k.setSharedKey(cfg.getKey(0));
    }
    byte buf[10];
    if((!getSerialNumber(buf, Configuration::IDLENGTH)) || (!k.setLocalId(buf,Configuration::IDLENGTH) ) || (!ecdh.init(buf, Configuration::IDLENGTH)) )
    {
        return;
    }
#ifdef ARDUINO_STM_NUCLEO_F103RB
#ifdef DEBUG
    Serial.println("Config valid");
#endif
    k.setSharedKey(cfg.getKey(0));
    Serial.println("Initiator starts authentication");
    if(!k.sendMessage(cfg.getId(0),payload,4))
    {
        Serial.println("Sending message failed.");
        return;
    }
#elif defined(ARDUINO_AVR_PROTRINKET3) || defined(ARDUINO_SAM_DUE)
    k.setMessageReceivedHandler(dataReceived);
#else
#error No device
#endif

#ifdef DEBUG
    Serial.println("ready");
#endif

}


void loop()
{
    if(ecdh.loop()==EcdhComm::AUTHENTICATION_OK)
    {
        Serial.println("Securely paired");
        k.setSharedKey(ecdh.getMasterKey());
        saveKey();
    }
#ifdef ARDUINO_STM_NUCLEO_F103RB
    if(k.loop()==Kryptoknight::AUTHENTICATION_AS_INITIATOR_OK)
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
#elif defined(ARDUINO_AVR_PROTRINKET3) || defined(ARDUINO_SAM_DUE)
    if(k.loop()==Kryptoknight::AUTHENTICATION_AS_PEER_OK)
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


void saveKey()
{
    cfg.setKey(0,ecdh.getRemoteId(), ecdh.getMasterKey());
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
