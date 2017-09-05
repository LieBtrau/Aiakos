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
    typedef bool(*RX_Function)(byte* data, byte& length);

    typedef enum
    {
        NO_AUTHENTICATION,
        AUTHENTICATION_OK,
        AUTHENTICATION_BUSY,
    }AUTHENTICATION_RESULT;

    BlePairing(TX_Function tx_func, RX_Function rx_func, bleControl* ble);
    virtual AUTHENTICATION_RESULT loop()=0;
protected:
    typedef enum
    {
        REMOTE_MAC,
        PASSCODE,
        RFID_KEY
    }PAIRING_MSGS;
    bool sendData(byte data[], byte id);
    bool receiveData(byte data[], byte id);
    bleControl* _ble;
    unsigned long _commTimeOut;
    byte rfidkey[4];
private:
    TX_Function _txfunc;
    RX_Function _rxfunc;
};

#endif // BLEPAIRING_H
