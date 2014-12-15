#ifndef IRPHY_H
#define IRPHY_H

#include "Arduino.h"

class IrPhy
{
public:
    IrPhy();
    void init();
    bool write(byte c);
    bool doTransmission();
private:
};


#endif // IRPHY_H
