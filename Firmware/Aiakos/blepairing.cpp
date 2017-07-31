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

BlePairing::BlePairing(TX_Function tx_func, RX_Function rx_func, bleControl *ble):
    _txfunc(tx_func),
    _rxfunc(rx_func),
    _messageBuffer(0),
    _ble(ble)
{
}

BlePairing::~BlePairing()
{
    if(_messageBuffer)
    {
        free(_messageBuffer);
    }
}

bool BlePairing::init()
{
    if(!_messageBuffer)
    {
        _messageBuffer=(byte*)malloc(MAX_MESSAGE_LEN);
        if(!_messageBuffer)
        {
            debug_println("Can't init.");
            return false;
        }
    }
    return true;
}

BlePairing::AUTHENTICATION_RESULT BlePairing::loop()
{

}

bool BlePairing::setRemoteBleAddress(byte* address)
{
    return _txfunc(address, 12);
}

bool BlePairing::getRemoteBleAddress(byte** address)
{
    byte length;
    return _rxfunc(address, length) && length==12;
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
