#ifndef KEYFOB_H
#define KEYFOB_H

#include "loradevice.h"
#include "blepairing.h"
#include "blecontrol.h"

class KeyFob : public LoRaDevice
{
public:
    KeyFob(byte ownAddress,
           Configuration* config,
           RH_RF95* prhLora,
           RH_Serial*prhSerial,
           byte buttonPin,
           byte cableDetectPin,
           bleControl* pble
           );
    bool setup();
    void loop();
    void eventPasscodeGenerated();
    void eventPasscodeInputRequested();
private:
    typedef enum
    {
        ECDHCOMM,
        BLE_BOND,
        UNKNOWN
    }SER_PROTOCOL;

    bool initBlePeripheral();

    byte BUTTON_PIN;
    Bounce pushButton;
    SER_PROTOCOL serProtocol;
    bleControl* _ble;
    BlePairing _blePair;
};
#endif // KEYFOB_H
