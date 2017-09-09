#ifndef GARAGECONTROLLER_H
#define GARAGECONTROLLER_H

#include "loradevice.h"


class GarageController : public LoRaDevice
{
public:
    GarageController(byte ownAddress, Configuration* config, RH_RF95* rhLora, RH_Serial *prhSerial, byte cableDetectPin);
    bool setup();
    void loop();
};

#endif // GARAGECONTROLLER_H
