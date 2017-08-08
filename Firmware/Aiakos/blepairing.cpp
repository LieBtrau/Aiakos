/* This class will execute the BLE-pairing based on the exchange of 6 digit PINs.
 *
 * !! Be warned that BLE pairing by design is known to have serious weaknesses !!
 *
 * If you want to know more, you can read:
 * https://lacklustre.net/bluetooth/Ryan_Bluetooth_Low_Energy_USENIX_WOOT.pdf
 * https://security.stackexchange.com/questions/100443/security-of-bluetooth-low-energy-ble-link-layer-encryption
 * https://eprint.iacr.org/2013/309.pdf
 *
 */

#include "blepairing.h"
#include "debug.h"

BlePairing::BlePairing(TX_Function tx_func, RX_Function rx_func, bleControl *ble, bool bIsPeripheral):
    _txfunc(tx_func),
    _rxfunc(rx_func),
    _ble(ble),
    _commTimeOut(0),
    _bIsPeripheral(bIsPeripheral)
{
    _state=bIsPeripheral? WAITING_FOR_START : WAITING_FOR_REMOTE_MAC;
}

BlePairing::~BlePairing()
{
}

bool BlePairing::init()
{
    return true;
}

bool BlePairing::PeripheralStartPairing()
{
    byte address[13];
    byte length;
    if( (!_bIsPeripheral) || (_state!=WAITING_FOR_START) )
    {
        return false;
    }
    //Peripheral get its MAC address from BLE module
    if(!_ble->getLocalMacAddress(address, length))
    {
        return false;
    }
    //MAC address is sent through (wired) serial connection to the central.
    if(!setRemoteBleAddress(address))
    {
        return false;
    }
    _commTimeOut=millis();
    _state=WAITING_FOR_PINCODE;
}


BlePairing::AUTHENTICATION_RESULT BlePairing::loop()
{
    if(millis()>_commTimeOut+15000)
    {
        debug_println("Timeout");
        _commTimeOut=millis();
        _state=_bIsPeripheral ? WAITING_FOR_START : WAITING_FOR_REMOTE_MAC;
        return NO_AUTHENTICATION;
    }
    switch(_state)
    {
    case WAITING_FOR_START:
        return NO_AUTHENTICATION;
    case WAITING_FOR_REMOTE_MAC:
        if(getRemoteBleAddress(_remoteBleAddress))
        {
            _commTimeOut=millis();
            _state=DETECT_BLE_PERIPHERAL;
        }
        return AUTHENTICATION_BUSY;
    case DETECT_BLE_PERIPHERAL:
        if(!_ble->findUnboundPeripheral((const char*)_remoteBleAddress))
        {
            _state=WAITING_FOR_REMOTE_MAC;
            return NO_AUTHENTICATION;
        }
        else
        {
            _state=PAIR_BLE_PERIPHERAL;
            return AUTHENTICATION_BUSY;
        }
    case PAIR_BLE_PERIPHERAL:
         if(_ble->secureConnect((const char*)_remoteBleAddress))
         {
            _state=WAITING_FOR_REMOTE_MAC;
            return AUTHENTICATION_OK;
         }
         _state=WAITING_FOR_REMOTE_MAC;
         return NO_AUTHENTICATION;
    case WAITING_FOR_PINCODE:
        return AUTHENTICATION_BUSY;
    case PASSCODE_SENT_PER:
        //Pairing ok here?
        return AUTHENTICATION_BUSY;
    default:
        return NO_AUTHENTICATION;
    }
}

void BlePairing::eventPasscodeGenerated()
{
    //Central reads passcode that has been generated inside the BLE module.
    unsigned long passcode=_ble->getPasscode();
    //Passcode is sent over the wired serial link to the peripheral
    if(_state==PAIR_BLE_PERIPHERAL && setPinCode(passcode))
    {
        _state=PASSCODE_SENT_CENT;
    }
}

void BlePairing::eventPasscodeInputRequested()
{
    unsigned long passcode;
    //Peripheral receives passcode over the wired serial link from the central.
    if( (_state!=WAITING_FOR_PINCODE) || (!getPinCode(passcode)))
    {
        return;
    }
    //Peripheral sends the passcode to its BLE module
    _ble->setPasscode(passcode);
    _state=PASSCODE_SENT_PER;
}


bool BlePairing::setRemoteBleAddress(byte *address)
{
    return _txfunc(address, 12);
}

bool BlePairing::getRemoteBleAddress(byte* address)
{
    byte length;
    return _rxfunc(&address, length) && length==12;
}

bool BlePairing::setPinCode(uint32_t pinCode)
{
    byte array[4];
    memcpy(array, &pinCode, 4);
    return _txfunc(array, 4);
}

bool BlePairing::getPinCode(uint32_t& pinCode)
{
    byte length;
    byte array[4];
    byte* array2=array;
    if(_rxfunc(&array2, length) && length==4)
    {
        memcpy(&pinCode, array2, 4);
    }
}

