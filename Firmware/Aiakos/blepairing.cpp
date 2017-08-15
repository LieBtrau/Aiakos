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
    _ble(ble),
    _commTimeOut(0)
{
}

BlePairing::~BlePairing()
{
}







