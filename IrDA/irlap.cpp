#include "irlap.h"

IrLAP::IrLAP():_irPhy()
{
}

void IrLAP::show(){
    _irPhy.show();
}

void IrLAP::init(){
    _irPhy.init();
}

bool IrLAP::send(byte* sendBuffer, byte byteCount){
    memcpy(_txBuffer, sendBuffer, byteCount);
    return _irPhy.send(_txBuffer, byteCount);
}
