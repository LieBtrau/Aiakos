#ifndef BLECONTROL_H
#define BLECONTROL_H

#include "Arduino.h"

class bleControl
{
public:
    bleControl();
    bool begin(bool bCentral);
    bool loop(void);
    bool getLocalMacAddress(byte* address, byte& length);
    bool findRemoteDevices();
};

#endif // BLECONTROL_H
