#ifndef KEYFOB_H
#define KEYFOB_H

#include "loradevice.h"

class KeyFob : public LoRaDevice
{
public:
    KeyFob(byte ownAddress, Configuration* config, RH_RF95* prhLora, RH_Serial*prhSerial, byte buttonPin, byte cableDetectPin);
    bool setup();
    void loop();
private:
    typedef enum
    {
        ECDHCOMM,
        BLE_BOND,
        UNKNOWN
    }SER_PROTOCOL;
    byte BUTTON_PIN;
    Bounce pushButton;
    SER_PROTOCOL serProtocol;
};
#endif // KEYFOB_H
