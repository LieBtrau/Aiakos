#include "loradevice.h"
#define DEBUG

namespace
{
RHReliableDatagram* pmgrLoRa;
RHReliableDatagram* pmgrSer;
byte peerAddress;
}
bool writeDataLoRa(byte* data, byte length);
bool readDataLoRa(byte** data, byte& length);
bool readDataSer(byte** data, byte& length);
bool writeDataSer(byte* data, byte length);
extern void print(const byte* array, byte length);

LoRaDevice::LoRaDevice(byte ownAddress, RH_RF95 *prhLora, RH_Serial *prhSerial, byte cableDetectPin):
    rhLoRa(prhLora),
    rhSerial(prhSerial),
    ecdh(&RNG, writeDataSer, readDataSer),
    mgrLoRa(*rhLoRa, ownAddress),
    mgrSer(*rhSerial, ownAddress),
    k(&RNG, writeDataLoRa, readDataLoRa),
    CABLE_DETECT_PIN(cableDetectPin)
{
    pmgrLoRa=&mgrLoRa;
    pmgrSer=&mgrSer;
}

bool LoRaDevice::setup()
{
    rhSerial->serial().begin(2400);
    if ((!mgrLoRa.init()))
    {
#ifdef DEBUG
        Serial.println("LoRa init failed");
#endif
        return false;
    }
    if ((!mgrSer.init()))
    {
#ifdef DEBUG
        Serial.println("Serial init failed");
#endif
        return false;
    }
    return true;
}

void LoRaDevice::setPeerAddress(byte address)
{
    peerAddress=address;
}


bool LoRaDevice::init()
{
    pinMode(CABLE_DETECT_PIN, INPUT_PULLUP);
    cableDetect.attach(CABLE_DETECT_PIN);
    cableDetect.interval(100); // interval in ms
    byte buf[10];
    return (getSerialNumber(buf, Configuration::IDLENGTH)
            && k.init(buf,Configuration::IDLENGTH)
            && ecdh.init(buf, Configuration::IDLENGTH));
}

bool writeDataSer(byte* data, byte length)
{
    Serial.print("Sending serial data...");
#ifdef DEBUG
    print(data, length);
#endif
    return pmgrSer->sendtoWait(data, length, peerAddress);
}

bool writeDataLoRa(byte* data, byte length)
{
#ifdef DEBUG
    Serial.println("Sending LoRa data: ");print(data, length);
#endif
    return pmgrLoRa->sendtoWait(data, length, peerAddress);
}

bool readDataSer(byte** data, byte& length)
{
    byte from;
    if (!pmgrSer->available())
    {
        return false;
    }
    if(!pmgrSer->recvfromAck(*data, &length, &from))
    {
        return false;
    }
    if(from != peerAddress)
    {
#ifdef DEBUG
        Serial.println("Sender doesn't match");
#endif
        return false;
    }
#ifdef DEBUG
    Serial.println("Received data: ");print(*data, length);
#endif
    return true;
}

bool readDataLoRa(byte** data, byte& length)
{
    byte from;
    if (!pmgrLoRa->available())
    {
        return false;
    }
    if(!pmgrLoRa->recvfromAck(*data, &length, &from))
    {
        return false;
    }
    if(from != peerAddress)
    {
#ifdef DEBUG
        Serial.println("Sender doesn't match");
#endif
        return false;
    }
    return true;
}

