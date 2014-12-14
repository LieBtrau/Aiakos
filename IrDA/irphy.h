#ifndef IRPHY_H
#define IRPHY_H

#include "Arduino.h"

class IrPhy
{
public:
    IrPhy();
    void init();
    static void isr();
    void sendByte(byte c);
private:
};


#endif // IRPHY_H
