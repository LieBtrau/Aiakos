#ifndef IRLAP_H
#define IRLAP_H

#include "Arduino.h"
#include "irphy.h"

class IrLAP
{
public:
    static const byte IRLAP_SIZE=IrPhy::ASYNC_WRAPPER_SIZE-14;
    IrLAP();
    void init();
    bool send(byte* sendBuffer, byte byteCount);
private:
    byte _txBuffer[IRLAP_SIZE];
    IrPhy _irPhy;
};

#endif // IRLAP_H
