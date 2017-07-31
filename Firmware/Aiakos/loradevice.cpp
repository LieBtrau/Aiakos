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
        debug_println("LoRa init failed");
        return false;
    }
    if ((!mgrSer.init()))
    {
        debug_println("Serial init failed");
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
    debug_print("Sending serial data...");
    debug_printArray(data, length);
    return pmgrSer->sendtoWait(data, length, peerAddress);
}

bool writeDataLoRa(byte* data, byte length)
{
    debug_print("Sending LoRa data: ");debug_printArray(data, length);
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
        debug_print("Sender doesn't match");
        return false;
    }
    debug_print("Received data: ");debug_printArray(*data, length);
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
        debug_print("Sender doesn't match");
        return false;
    }
    return true;
}

