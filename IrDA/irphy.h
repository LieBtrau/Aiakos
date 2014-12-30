#ifndef IRPHY_H
#define IRPHY_H

#include "Arduino.h"

class IrPhy
{
public:
    static const byte ASYNC_WRAPPER_SIZE=100;
    static const word NINE_ONES_MAX=1370;
    static const word EIGHT_ONES_MAX=1230;
    static const word SEVEN_ONES_MAX=1090;
    static const word SIX_ONES_MAX=950;
    static const word FIVE_ONES_MAX=810;
    static const word FOUR_ONES_MAX=670;
    static const word THREE_ONES_MAX=530;
    static const word TWO_ONES_MAX=390;
    static const word ONE_ONES_MAX=250;
    static const word ZERO_ONES_MAX=110;
    static const word MINIMUM_GAP=90;
    static const word MAXIMUM_GAP=0xFFFF;
    IrPhy();
    void init();
    bool send(byte* sendBuffer, byte byteCount);
    bool sendRaw(byte* sendBuffer, byte byteCount);
    bool sendingDone();
    void show();
private:
    static const byte XBOF=0xFF;
    static const byte BOF=0xC0;
    static const byte EOF_FLAG=0xC1;
    static const byte CE=0x7D;
    void processShiftRegister(word sr);
    void startTx(byte* buffer, byte size);
    byte _sendPacket[ASYNC_WRAPPER_SIZE];
};


#endif // IRPHY_H
