#ifndef BLECONTROL_H
#define BLECONTROL_H

#include "Arduino.h"

class bleControl
{
public:
    typedef enum
    {
        ST_NOTCONNECTED,
        ST_PASS_GENERATED,
        ST_BONDED
    }CONNECT_STATE;
    bleControl();
    bool begin(bool bCentral);
    bool loop(void);
    bool getLocalMacAddress(byte* address, byte& length);
    bool findUnboundPeripheral(const char *remoteBtAddress);
    CONNECT_STATE secureConnect(const char* remoteBtAddress, CONNECT_STATE state);
    unsigned long getPasscode();
    void disconnect();
};

#endif // BLECONTROL_H
