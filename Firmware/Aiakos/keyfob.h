#ifndef KEYFOB_H
#define KEYFOB_H

#include "loradevice.h"
#include "blepairingperipheral.h"
#include "blecontrol.h"
#include "STM32Sleep.h"

class KeyFob : public LoRaDevice
{
public:
    KeyFob(byte ownAddress,
           Configuration* config,
           RH_RF95* prhLora,
           RH_Serial*prhSerial,
           byte buttonPin,
           byte cableDetectPin,
           bleControl* pble,
           byte tonePin
           );
    bool setup();
    void loop();
    void event(bleControl::EVENT ev);
    void rfidEvent(byte* value, byte &length);
    void alertEvent(byte* value, byte &length);
private:
    typedef enum
    {
        ECDHCOMM,
        BLE_BOND,
        UNKNOWN
    }SER_PROTOCOL;

    bool initBlePeripheral();
    bool storeBleData();
    void sleep();
    bool verifyRfidKey(byte *value);
    byte buttonPin;
    byte tonePin;
    Bounce pushButton;
    SER_PROTOCOL serProtocol;
    bleControl* _ble;
    blePairingPeripheral _blePair;
};
#endif // KEYFOB_H
