#ifndef KEYFOB_H
#define KEYFOB_H

#include "loradevice.h"

class KeyFob : public LoRaDevice
{
public:
    KeyFob(byte ownAddress, Configuration* config, RH_RF95* prhLora, RH_Serial*prhSerial);
    void setup();
    void loop();
private:
    typedef enum
    {
        ECDHCOMM,
        BLE_BOND,
        UNKNOWN
    }SER_PROTOCOL;
    const byte BUTTON_PIN=25;
    Bounce pushButton;
    SER_PROTOCOL serProtocol;
};
#endif // KEYFOB_H
