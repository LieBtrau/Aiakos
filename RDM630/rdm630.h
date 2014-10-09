#ifndef RDM630_H
#define RDM630_H

#include "Arduino.h"
#include "SoftwareSerial.h"

SoftwareSerial RFID(2, 3); // RX and TX

class rdm630
{
public:
    rdm630(byte yPinRx, byte yPinTx);
private:
    typedef enum{
        WAITING_FOR_STX,
        READING_DATA,
        DATA_VALID
    }state;
    byte AsciiCharToNum(byte data);
    state dataParser(state s, byte c);

};

#endif // RDM630_H
