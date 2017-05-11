#include "keyfob.h"
#define DEBUG

namespace {
    byte payload[4]={0xFE, 0xDC, 0xBA, 0x98};
    Configuration* cfg;
}

KeyFob::KeyFob(byte ownAddress,
        byte peerAddress,
        Configuration* config,
        RH_RF95 *prhLora, RH_Serial *prhSerial): LoRaDevice(ownAddress, peerAddress, config, prhLora, prhSerial)
{
    CABLE_DETECT_PIN=6;
    pushButton = Bounce();
    cfg=config;
}

void KeyFob::setup()
{
    LoRaDevice::setup();
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pushButton.attach(BUTTON_PIN);
    pushButton.interval(100); // interval in ms
    LoRaDevice::init();
}


void KeyFob::loop()
{
    cableDetect.update();
    pushButton.update();
    if(!cableDetect.read())
    {
        //Secure pairing mode
        if(pushButton.fell())
        {
#ifdef DEBUG
            Serial.println("Starting pairing...");
#endif
            k.reset();
            if(!ecdh.startPairing())
            {
                Serial.println("Sending message failed.");
                return;
            }
        }
        switch(ecdh.loop())
        {
        case EcdhComm::AUTHENTICATION_OK:
#ifdef DEBUG
            Serial.println("Securely paired");
#endif
            cfg->addKey(ecdh.getRemoteId(), ecdh.getMasterKey());
            break;
        case EcdhComm::NO_AUTHENTICATION:
        case EcdhComm::AUTHENTICATION_BUSY:
            break;
        }
    }else
    {
        //Authenticating mode
        if(pushButton.fell())
        {
            ecdh.reset();
#ifdef DEBUG
            Serial.println("Initiator starts authentication");
#endif
            if(!k.sendMessage(payload,sizeof(payload), cfg->getDefaultId(), cfg->getIdLength(), cfg->getDefaultKey()))
            {
                Serial.println("Sending message failed.");
                return;
            }
        }
        if(k.loop()==KryptoKnightComm::AUTHENTICATION_AS_INITIATOR_OK)
        {
            Serial.println("Message received by peer and acknowledged");
        }
    }
}
