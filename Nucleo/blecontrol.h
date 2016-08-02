#ifndef BLECONTROL_H
#define BLECONTROL_H

#include "Arduino.h"

class bleControl
{
public:
    bleControl();
    bool begin(bool bCentral);
    bool loop(void);
};

#endif // BLECONTROL_H
