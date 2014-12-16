#ifndef IRPHY_H
#define IRPHY_H

#include "Arduino.h"

class IrPhy
{
public:
    static const byte ASYNC_WRAPPER_SIZE=100;
    IrPhy();
    void init();
    bool send(byte* sendBuffer, byte byteCount);
private:
    static const byte XBOF=0xFF;
    static const byte BOF=0xC0;
    static const byte EOF_FLAG=0xC1;
    static const byte CE=0x7D;
};


#endif // IRPHY_H
