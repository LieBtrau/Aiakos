#include "rdm630.h"
#include <string.h>

rdm630::rdm630(byte yPinRx, byte yPinTx):
    _rfid(yPinRx, yPinTx),
    _s(WAITING_FOR_STX)
{
}

void rdm630::begin(){
    _rfid.begin(9600);    // start serial to RFID reader
}

bool rdm630::available(){
    if (_rfid.available() > 0)
    {
        _s=dataParser(_s,_rfid.read());
        return(_s==DATA_VALID);
    }
    return false;
}

void rdm630::getData(byte* data, byte& length){
    length=sizeof(_data);
    memcpy(data,_data,sizeof(_data));
}

byte rdm630::AsciiCharToNum(byte data){
    return (data > '9'?data-'0'-7:data-'0');
}

rdm630::state rdm630::dataParser(state s, byte c){
    switch(s){
    case WAITING_FOR_STX:
    case DATA_VALID:
        if(c==STX){
            _iNibbleCtr=-1;
            return READING_DATA;
        }
        break;
    case READING_DATA:
        if(++_iNibbleCtr<12){
            _data[_iNibbleCtr>>1]=((_iNibbleCtr & 0x1)==0?AsciiCharToNum(c)<<4 : _data[_iNibbleCtr>>1] + AsciiCharToNum(c));
            return READING_DATA;
        }
        if(c!=ETX){     //Expected end character, but got something else
            return WAITING_FOR_STX;
        }
        for(int i=0;i<5;i++){
            _data[5]^=_data[i];
        }
        if(_data[5]!=0){ //Checksum doesn't match
            return WAITING_FOR_STX;
        }
        return DATA_VALID;
    default:
        return WAITING_FOR_STX;
    }
    return WAITING_FOR_STX;
}
