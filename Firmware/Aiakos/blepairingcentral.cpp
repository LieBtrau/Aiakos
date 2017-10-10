#include "blepairingcentral.h"

void blePairingCentral::init(byte key[])
{
    memcpy(rfidkey, key, _rfidKeyLength);
}

byte* blePairingCentral::getRemoteBleAddress()
{
    return _remoteBleAddress;
}

void blePairingCentral::eventPasscodeGenerated()
{
    //Central reads passcode that has been generated inside the BLE module.
    unsigned long passcode=_ble->getPasscode();
    //Passcode is sent over the wired serial link to the peripheral
    byte data[4];
    for(byte i=0;i<4;i++)
    {
        data[i]=passcode & 0xFF;
        passcode>>=8;
    }
    if(_state==PAIR_BLE_PERIPHERAL && sendData(data, 4, PASSCODE))
    {
        pinCodeSent=true;
    }
}

blePairingCentral::AUTHENTICATION_RESULT blePairingCentral::loop()
{
    byte length=sizeof(_remoteBleAddress);
    _ble->loop();
    if(millis()>_commTimeOut+7000)
    {
        debug_println("Blepairing Timeout");
        _commTimeOut=millis();
        _state=WAITING_FOR_REMOTE_MAC;
        return NO_AUTHENTICATION;
    }
    switch(_state)
    {
    case WAITING_FOR_REMOTE_MAC:
        if(receiveData(_remoteBleAddress, length, REMOTE_MAC))
        {
            _commTimeOut=millis();
            _state=SENDING_RFID_CODE;
            debug_print("Remote MAC-address received:");
            debug_printArray(_remoteBleAddress, 6);
        }
        return AUTHENTICATION_BUSY;
    case SENDING_RFID_CODE:
        if(!sendData(rfidkey, _rfidKeyLength, RFID_KEY))
        {
            _state=WAITING_FOR_REMOTE_MAC;
            return NO_AUTHENTICATION;
        }
        _state=DETECT_BLE_PERIPHERAL;
        return AUTHENTICATION_BUSY;
    case DETECT_BLE_PERIPHERAL:
        //Unbonding is performed in this step
        if(!_ble->findUnboundPeripheral(_remoteBleAddress))
        {
            _state=WAITING_FOR_REMOTE_MAC;
            return NO_AUTHENTICATION;
        }
        debug_println("BLE device found.");
        _state=PAIR_BLE_PERIPHERAL;
        pinCodeSent=false;
        return AUTHENTICATION_BUSY;
    case PAIR_BLE_PERIPHERAL:
        if(!_ble->secureConnect(_remoteBleAddress))
        {
            if(!pinCodeSent)
            {
                debug_println("Pincode not sent to peripheral.");
            }
            _state=WAITING_FOR_REMOTE_MAC;
            return NO_AUTHENTICATION;
        }
        debug_println("BLE device connected.");
        _state=WAITING_FOR_REMOTE_MAC;
        return AUTHENTICATION_OK;
    default:
        return NO_AUTHENTICATION;
    }
}

