#include "blepairingcentral.h"

void blePairingCentral::eventPasscodeGenerated()
{
    //Central reads passcode that has been generated inside the BLE module.
    unsigned long passcode=_ble->getPasscode();
    //Passcode is sent over the wired serial link to the peripheral
    if(_state==PAIR_BLE_PERIPHERAL && setPinCode(passcode))
    {
        debug_print("Passcode sent & ack'd.");
        pinCodeSent=true;
    }
}

bool blePairingCentral::setPinCode(uint32_t pinCode)
{
    byte array[4];
    memcpy(array, &pinCode, 4);
    return _txfunc(array, 4);
}

bool blePairingCentral::getRemoteBleAddress(byte* address)
{
    byte length=6;
    if(!address)
    {
        return false;
    }
    return _rxfunc(&address, length) && length==6;
}

blePairingCentral::AUTHENTICATION_RESULT blePairingCentral::loop()
{

    _ble->loop();
    if(millis()>_commTimeOut+7000)
    {
        debug_println("Timeout");
        _commTimeOut=millis();
        _state=WAITING_FOR_REMOTE_MAC;
        return NO_AUTHENTICATION;
    }
    switch(_state)
    {
    case WAITING_FOR_REMOTE_MAC:
        if(getRemoteBleAddress(_remoteBleAddress))
        {
            _commTimeOut=millis();
            _state=DETECT_BLE_PERIPHERAL;
            debug_print("Remote MAC-address received:");
            debug_printArray(_remoteBleAddress, 6);
        }
        return AUTHENTICATION_BUSY;
    case DETECT_BLE_PERIPHERAL:
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

