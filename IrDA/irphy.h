#ifndef IRPHY_H
#define IRPHY_H

#include "Arduino.h"

class IrPhy
{
public:
    static const byte ASYNC_WRAPPER_SIZE=100;
    static const int NINE_ONES_MAX=1370;
    static const int EIGHT_ONES_MAX=1230;
    static const int SEVEN_ONES_MAX=1090;
    static const int SIX_ONES_MAX=950;
    static const int FIVE_ONES_MAX=810;
    static const int FOUR_ONES_MAX=670;
    static const int THREE_ONES_MAX=530;
    static const int TWO_ONES_MAX=390;
    static const int ONE_ONES_MAX=250;
    static const int ZERO_ONES_MAX=110;
    static const int MINIMUM_GAP=90;
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
    void startTx(byte* buffer, byte size);
    byte _sendPacket[IrPhy::ASYNC_WRAPPER_SIZE];
};


#endif // IRPHY_H
