#ifndef LORADEVICE_H
#define LORADEVICE_H

#include <RHReliableDatagram.h> //for wireless comm
#include <RH_RF95.h>            //for wireless comm
#include <RH_Serial.h>          //for wired comm
#include <SPI.h>                //for wireless comm
#include "kryptoknightcomm.h"   //for authentication
#include "ecdhcomm.h"           //for secure pairing
#include "configuration.h"      //for non-volatile storage of parameters
#include "cryptohelper.h"       //for unique serial numbers & true random number generators
#include <Bounce2.h>            //for switch debouncing


class LoRaDevice
{
public:
    virtual void setup();
    virtual void loop()=0;
protected:
    LoRaDevice(byte ownAddress, byte peerAddress, Configuration* cfg, RH_RF95* prhLora, RH_Serial* prhSerial);
    void init();
    KryptoKnightComm k;
    EcdhComm ecdh;
    Bounce cableDetect;
    RH_RF95* rhLoRa;
    RH_Serial* rhSerial;
    RHReliableDatagram mgrLoRa;
    RHReliableDatagram mgrSer;
    byte CABLE_DETECT_PIN;
    byte localAddress;
    byte peerAddress;
};

#endif // LORADEVICE_H
