#include "irlap.h"

IrLAP::IrLAP():_irPhy()
{
}

void IrLAP::init(){
    _irPhy.init();
}

bool IrLAP::send(byte* sendBuffer, byte byteCount){
    byte packetDataCnt=0;
    memcpy(_txBuffer, sendBuffer, byteCount);
    return _irPhy.send(_txBuffer, byteCount);
}
