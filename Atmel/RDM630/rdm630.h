/*  RDM630 RFID reader library
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

#ifndef RDM630_H
#define RDM630_H

#include "Arduino.h"
#include "SoftwareSerial.h"

class rdm630
{
public:
    rdm630(byte yPinRx, byte yPinTx);
    void begin();
    bool available();
    void getData(byte* data, byte& length);
private:
    typedef enum{
        WAITING_FOR_STX,
        READING_DATA,
        DATA_VALID
    }state;
    static const int STX=2;
    static const int ETX=3;

    byte AsciiCharToNum(byte data);
    state dataParser(state s, byte c);

    SoftwareSerial _rfid;
    state _s;
    int _iNibbleCtr;
    byte _data[6];
};

#endif // RDM630_H
