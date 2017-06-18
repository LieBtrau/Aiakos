#include "keyfob.h"
#define DEBUG

namespace {
byte payload[4]={0xFE, 0xDC, 0xBA, 0x98};
Configuration* cfg;
}

KeyFob::KeyFob(byte ownAddress,
               Configuration* config,
               RH_RF95 *prhLora, RH_Serial *prhSerial):
    LoRaDevice(ownAddress, prhLora, prhSerial),
    serProtocol(ECDHCOMM)
{
    CABLE_DETECT_PIN=6;
    pushButton = Bounce();
    cfg=config;
    setPeerAddress(1);
}

void KeyFob::setup()
{
    LoRaDevice::setup();
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pushButton.attach(BUTTON_PIN);
    pushButton.interval(100); // interval in ms
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
            Serial.println("Starting ECDH pairing...");
#endif
            serProtocol=ECDHCOMM;
            setPeerAddress(1);
            k.reset();
            if(!ecdh.startPairing())
            {
#ifdef DEBUG
                Serial.println("Starting BLE pairing...");
#endif
                serProtocol=BLE_BOND;
                setPeerAddress(3);
                return;
            }
        }
        switch(serProtocol)
        {
        case ECDHCOMM:
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
            case EcdhComm::UNKNOWN_DATA:
                serProtocol=UNKNOWN;
                break;
            }
            break;
        case UNKNOWN:
            //find out the correct protocol
            break;
        case BLE_BOND:
            break;
        }
    }else
    {
        //Authenticating remote peer mode
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
