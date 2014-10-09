#ifndef RDM630_H
#define RDM630_H

#include "Arduino.h"
#include "SoftwareSerial.h"

SoftwareSerial RFID(2, 3); // RX and TX

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

    int _iNibbleCtr;
    state _s;
    byte _data[6];
    SoftwareSerial _rfid;
};

#endif // RDM630_H
