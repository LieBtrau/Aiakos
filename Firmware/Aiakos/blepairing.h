#ifndef BLEPAIRING_H
#define BLEPAIRING_H

#include <RHReliableDatagram.h> //for wired comm
#include <RH_Serial.h>          //for wired comm
#include "blecontrol.h"
#include "debug.h"

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
    }AUTHENTICATION_RESULT;

    BlePairing(TX_Function tx_func, RX_Function rx_func, bleControl* ble);
    ~BlePairing();
    virtual AUTHENTICATION_RESULT loop()=0;
protected:
    TX_Function _txfunc;
    RX_Function _rxfunc;
    bleControl* _ble;
    unsigned long _commTimeOut;
};

#endif // BLEPAIRING_H
