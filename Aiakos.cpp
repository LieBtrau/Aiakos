// Do not remove the include below
#include "Aiakos.h"
#include "SoftwareSerial.h"

SoftwareSerial RFID(2, 3); // RX and TX
uint8_t AsciiCharToNum(byte data);
state dataParser(state s, byte c);

const int STX=2;
const int ETX=3;

int iNibbleCtr;
byte data[6];
state s;

void setup()
{
    RFID.begin(9600);    // start serial to RFID reader
    Serial.begin(9600);  // start serial to PC
    s=WAITING_FOR_STX;
}


void loop() {
    if (RFID.available() > 0)
    {
        s=dataParser(s,RFID.read());
        if(s==DATA_VALID){
            Serial.println("Data valid");
            for(int i=0;i<5;i++){
                Serial.print(data[i],HEX);
            }
            Serial.println();
        }
    }
}

state dataParser(state s, byte c){
    switch(s){
    case WAITING_FOR_STX:
    case DATA_VALID:
        if(c==STX){
            iNibbleCtr=-1;
            return READING_DATA;
        }
        break;
    case READING_DATA:
        if(++iNibbleCtr<12){
            data[iNibbleCtr>>1]=((iNibbleCtr & 0x1)==0?AsciiCharToNum(c)<<4 : data[iNibbleCtr>>1] + AsciiCharToNum(c));
            return READING_DATA;
        }
        if(c!=ETX){     //Expected end character, but got something else
            return WAITING_FOR_STX;
        }
        for(int i=0;i<5;i++){
            data[5]^=data[i];
        }
        if(data[5]!=0){ //Checksum doesn't match
            return WAITING_FOR_STX;
        }
        return DATA_VALID;
    default:
        return WAITING_FOR_STX;
    }
}

uint8_t AsciiCharToNum(byte data) {
    return (data > '9'?data-'0'-7:data-'0');
}
