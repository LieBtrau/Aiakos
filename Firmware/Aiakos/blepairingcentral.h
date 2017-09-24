#ifndef BLEPAIRINGCENTRAL_H
#define BLEPAIRINGCENTRAL_H

#include "blepairing.h"

class blePairingCentral : BlePairing
{
public:
    blePairingCentral(TX_Function tx_func, RX_Function rx_func, bleControl* ble, byte rfidKeyLength):
    BlePairing(tx_func, rx_func, ble, rfidKeyLength){}
    void eventPasscodeGenerated();
    AUTHENTICATION_RESULT loop();
    byte* getRemoteBleAddress();
    bool init(byte key[]);
private:
    typedef enum
    {
        WAITING_FOR_REMOTE_MAC,
        SENDING_RFID_CODE,
        DETECT_BLE_PERIPHERAL,
        PAIR_BLE_PERIPHERAL,
    }AUTHENTICATION_STATE;
    byte _remoteBleAddress[6];
    AUTHENTICATION_STATE _state=WAITING_FOR_REMOTE_MAC;
    bool pinCodeSent=false;
};

#endif // BLEPAIRINGCENTRAL_H
