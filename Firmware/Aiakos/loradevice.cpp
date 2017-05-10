#include "loradevice.h"

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

LoRaDevice::LoRaDevice(byte ownAddress, byte remoteAddress, Configuration *cfg, RH_RF95 *prhLora, RH_Serial *prhSerial):
    localAddress(ownAddress),
    peerAddress(peerAddress),
    rhLoRa(prhLora),
    rhSerial(prhSerial),
    ecdh(&RNG, writeDataSer, readDataSer),
    mgrLoRa(*rhLoRa, ownAddress),
    mgrSer(*rhSerial, ownAddress),
    k(&RNG, writeDataLoRa, readDataLoRa)
{
    pmgrLoRa=&mgrLoRa;
    pmgrSer=&mgrSer;
    peerAddress=remoteAddress;
}

void LoRaDevice::setup()
{
    rhSerial->serial().begin(2400);
#ifdef DEBUG
    Serial.begin(9600);
    //Serial port will only be connected in debug mode
    while (!Serial) ; // Wait for serial port to be available
#endif
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
}

void LoRaDevice::init()
{
    pinMode(CABLE_DETECT_PIN, INPUT_PULLUP);
    cableDetect.attach(CABLE_DETECT_PIN);
    cableDetect.interval(100); // interval in ms
    byte buf[10];
    if((!getSerialNumber(buf, Configuration::IDLENGTH))
            || (!k.init(buf,Configuration::IDLENGTH) )
            || (!ecdh.init(buf, Configuration::IDLENGTH)) )
    {
#ifdef DEBUG
        Serial.println("Security init failed");
#endif
        return;
    }

#ifdef DEBUG
    Serial.println("ready");
#endif

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

