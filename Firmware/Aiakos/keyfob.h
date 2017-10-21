#ifndef KEYFOB_H
#define KEYFOB_H

#ifndef ARDUINO_SAM_DUE

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
    void getInitialPinStates();
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
    typedef enum
    {
        NONE,
        PAIRING,
        NORMAL
    }LOOP_MODE;
    typedef enum
    {
        NO_SOURCE,
        PUSHBUTTON,
        BLE_CONNECTION
    }WAKEUP_SOURCE;
    typedef enum
    {
        NOT_STARTED,
        START,
        RUNNING,
        STOPPED
    }ALERT_STATE;
    typedef struct
    {
        const byte PULSE_COUNT=1;
        const unsigned long PULSE_LENGTH=500;
        const unsigned long PULSE_PERIOD=1000;
        const unsigned long TONE_FREQUENCY=3120;
        const byte ALERT_VALUE=0xBB;
        unsigned long ulTimer;
        ALERT_STATE state;
        byte pulseCounter;
    }alert_level;
    bool initBlePeripheral(bool &rfidKeyVerified);
    bool storeBleData();
    void sleep();
    bool verifyRfidKey(byte *value);
    byte buttonPin;
    byte tonePin;
    Bounce pushButton;
    SER_PROTOCOL serProtocol;
    bleControl* _ble;
    blePairingPeripheral _blePair;
    LOOP_MODE loopmode;
    WAKEUP_SOURCE wakeupsource;
    alert_level alert;
};
#endif // KEYFOB_H
#endif
