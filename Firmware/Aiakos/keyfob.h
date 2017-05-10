#ifndef KEYFOB_H
#define KEYFOB_H

#include "loradevice.h"

class KeyFob : public LoRaDevice
{
public:
    KeyFob(byte ownAddress, byte peerAddress, Configuration* config, RH_RF95* prhLora, RH_Serial*prhSerial);
    void setup();
    void loop();
private:
    const byte BUTTON_PIN=25;
    Bounce pushButton;
};

#endif // KEYFOB_H
