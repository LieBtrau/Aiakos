#ifndef BLEPAIRING_H
#define BLEPAIRING_H

#include <RHReliableDatagram.h> //for wired comm
#include <RH_Serial.h>          //for wired comm
#include "blecontrol.h"

class BlePairing
{
public:
    typedef bool(*TX_Function)(byte* data, byte length);
    typedef bool(*RX_Function)(byte** data, byte& length);

    typedef enum
    {
        NO_AUTHENTICATION,
        AUTHENTICATION_OK,
        AUTHENTICATION_BUSY,
        UNKNOWN_DATA
    }AUTHENTICATION_RESULT;

    BlePairing(TX_Function tx_func, RX_Function rx_func, bleControl* ble, bool bIsPeripheral);
    ~BlePairing();
    bool init();
    bool PeripheralStartPairing();
    AUTHENTICATION_RESULT loop();
    void eventPasscodeGenerated();
    void eventPasscodeInputRequested();
private:
    typedef enum
    {
        WAITING_FOR_START,
        WAITING_FOR_PINCODE,
        WAITING_FOR_REMOTE_MAC,
        DETECT_BLE_PERIPHERAL,
        PAIR_BLE_PERIPHERAL,
        PASSCODE_SENT_CENT,
        PASSCODE_SENT_PER
    }AUTHENTICATION_STATE;
    bool setRemoteBleAddress(byte *address);
    bool getRemoteBleAddress(byte *address);
    bool setPinCode(uint32_t pinCode);
    bool getPinCode(uint32_t& pinCode);
    TX_Function _txfunc;
    RX_Function _rxfunc;
    bleControl* _ble;
    unsigned long _commTimeOut;
    AUTHENTICATION_STATE _state;
    bool _bIsPeripheral;
    byte _remoteBleAddress[13];
};

#endif // BLEPAIRING_H
