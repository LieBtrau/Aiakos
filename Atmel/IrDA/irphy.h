/*  IrDA library
    Copyright (C) 2014  Christoph Tack

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
    bool send(const byte* sendBuffer, byte byteCount);
    bool sendRaw(const byte* sendBuffer, byte byteCount);
    bool recv(byte* buffer, byte& length);
    bool recvraw(byte* buffer, byte& length);
    bool sendingDone();
private:
    static const byte XBOF=0xFF;
    static const byte BOF=0xC0;
    static const byte EOF_FLAG=0xC1;
    static const byte CE=0x7D;
    typedef enum{STATE_A, STATE_B, STATE_C, STATE_D} RXSTATE;
    bool processShiftRegister(word sr, byte &data);
    void startTx(byte* buffer, byte size);
    bool pop(word &data);
    RXSTATE _rxState;
    byte _txPacket[ASYNC_WRAPPER_SIZE];
    byte _rxPacket[ASYNC_WRAPPER_SIZE];
    byte _rxCtr;
};

void push(word value);
typedef enum{RX_MODE, TX_MODE} TIMER_MODE;
void setTimerMode(TIMER_MODE tm);

#endif // IRPHY_H
