#include "blepairingperipheral.h"


bool blePairingPeripheral::startPairing()
{
    byte address[6];
    byte length;
    if( (_state!=WAITING_FOR_START) )
    {
        return false;
    }
    //Peripheral get its MAC address from BLE module
    if(!_ble->getLocalMacAddress(address, length) || length!=6)
    {
        return false;
    }
    //MAC address is sent through (wired) serial connection to the central.
    if(!setRemoteBleAddress(address, length))
    {
        return false;
    }
    //Use 100ms beacon interval, so that connecting works smoother.
    _ble->startUndirectedAdvertizement(100);
    _commTimeOut=millis();
    bleRequestsPin=false;
    bondingEstablished=false;
    pincode=0;
    debug_println("MAC-address sent");
    _state=WAITING_FOR_PINCODE;
}

bool blePairingPeripheral::setRemoteBleAddress(byte *address, byte length)
{
    return _txfunc(address, length);
}

bool blePairingPeripheral::getPinCode(uint32_t& pinCode)
{
    byte length=4;
    byte array[4];
    byte* array2=array;
    if(_rxfunc(&array2, length) && length==4)
    {
        memcpy(&pinCode, array2, 4);
        return true;
    }
    return false;
}

void blePairingPeripheral::eventPasscodeInputRequested()
{
    bleRequestsPin=true;
    debug_println("BLE requests pincode");
}

void blePairingPeripheral::eventBondingEstablished()
{
    bondingEstablished=true;
    debug_println("Bonding established");
}

blePairingPeripheral::AUTHENTICATION_RESULT blePairingPeripheral::loop()
{
    _ble->loop();
    if(millis()>_commTimeOut+7000)
    {
        debug_println("Timeout");
        _commTimeOut=millis();
        _state = WAITING_FOR_START;
        return NO_AUTHENTICATION;
    }
    switch(_state)
    {
    case WAITING_FOR_START:
        return NO_AUTHENTICATION;
    case WAITING_FOR_PINCODE:
        if(getPinCode(pincode))
        {
            debug_print("Passcode received: ");
            debug_println(pincode, DEC);
            _state=PINCODE_RECEIVED;
        }
        return AUTHENTICATION_BUSY;
    case PINCODE_RECEIVED:
        if(bleRequestsPin)
        {
            //Peripheral sends the passcode to its BLE module
            _ble->setPasscode(pincode);
            debug_println("Pincode sent to BLE module");
            _state=PINCODE_SENT;
        }
        return AUTHENTICATION_BUSY;
    case PINCODE_SENT:
        if(bondingEstablished)
        {
            _state=WAITING_FOR_START;
            _ble->disconnect();
            return AUTHENTICATION_OK;
        }
        return AUTHENTICATION_BUSY;
    default:
        return NO_AUTHENTICATION;
    }
}





