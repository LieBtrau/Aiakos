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

    BlePairing(TX_Function tx_func, RX_Function rx_func, bleControl* ble);
    ~BlePairing();
    bool init();
    AUTHENTICATION_RESULT loop();
private:
    bool setRemoteBleAddress(byte* address);
    bool getRemoteBleAddress(byte **address);
    bool setPinCode(uint32_t pinCode);
    bool getPinCode(uint32_t& pinCode);
    TX_Function _txfunc;
    RX_Function _rxfunc;
    static const byte MAX_MESSAGE_LEN=20;
    byte* _messageBuffer;
    bleControl* _ble;
};

#endif // BLEPAIRING_H
