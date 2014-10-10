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
