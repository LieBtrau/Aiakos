#ifndef GARAGECONTROLLER_H
#define GARAGECONTROLLER_H

#include "loradevice.h"


class GarageController : public LoRaDevice
{
public:
    GarageController(byte ownAddress, byte peerAddress, Configuration* config, RH_RF95* rhLora, RH_Serial *prhSerial);
    void setup();
    void loop();
private:
    const byte PULSE_PIN=5;
};

#endif // GARAGECONTROLLER_H
