#include "blepairingperipheral.h"


bool blePairingPeripheral::startPairing()
{
    byte rmac[6];
    byte length;
    if( (_state!=WAITING_FOR_START) )
    {
        return false;
    }
    if(!_ble->unbond())
    {
        return false;
    }
    //Use 100ms beacon interval, so that connecting works smoother.
    _ble->startAdvertizement(100);
    //Peripheral get its MAC address from BLE module
    if(!_ble->getLocalMacAddress(rmac, length) || length!=6)
    {
        return false;
    }
    //MAC address is sent through (wired) serial connection to the central.
    if(!sendData(rmac,length,REMOTE_MAC))
    {
        return false;
    }
    _commTimeOut=millis();
    bleRequestsPass=false;
    bondingBonded=false;
    passcode=0;
    debug_println("MAC-address sent");
    _state=WAITING_FOR_RFID_KEY;
}

void blePairingPeripheral::eventPasscodeInputRequested()
{
    bleRequestsPass=true;
    debug_println("BLE requests pincode");
}

void blePairingPeripheral::eventBondingBonded()
{
    bondingBonded=true;
    debug_println("Bonding established");
}

blePairingPeripheral::AUTHENTICATION_RESULT blePairingPeripheral::loop()
{
    byte length;
    _ble->loop();
    if(millis()>_commTimeOut+7000)
    {
        debug_println("Blepairing Timeout");
        _commTimeOut=millis();
        _state = WAITING_FOR_START;
        return NO_AUTHENTICATION;
    }
    switch(_state)
    {
    case WAITING_FOR_START:
        return NO_AUTHENTICATION;
    case WAITING_FOR_RFID_KEY:
        length=5;
        if(receiveData(rfidkey,length, RFID_KEY))
        {
            _state=WAITING_FOR_PASSCODE;
        }
        return AUTHENTICATION_BUSY;
    case WAITING_FOR_PASSCODE:
        byte passArray[4];
        length=4;
        if(receiveData(passArray, length, PASSCODE))
        {
            for(byte i=0;i<4;i++)
            {
                passcode<<=8;
                passcode+=passArray[3-i];
            }
            debug_print("Passcode received: ");
            debug_println(passcode, DEC);
            _state=PASSCODE_RECEIVED;
        }
        return AUTHENTICATION_BUSY;
    case PASSCODE_RECEIVED:
        if(bleRequestsPass)
        {
            //Peripheral sends the passcode to its BLE module
            _ble->setPasscode(passcode);
            debug_println("Passcode sent to BLE module");
            _state=PASSCODE_SENT;
        }
        return AUTHENTICATION_BUSY;
    case PASSCODE_SENT:
        if(bondingBonded)
        {
            _state=WAITING_FOR_START;
            return AUTHENTICATION_OK;
        }
        return AUTHENTICATION_BUSY;
    default:
        return NO_AUTHENTICATION;
    }
}

bool blePairingPeripheral::getRfidKey(byte key[])
{
    if(!key)
    {
        return false;
    }
    memcpy(key, rfidkey, _rfidKeyLength);
    return true;
}





