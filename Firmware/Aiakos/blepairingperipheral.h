#ifndef BLEPAIRINGPERIPHERAL_H
#define BLEPAIRINGPERIPHERAL_H
#include "blepairing.h"

class blePairingPeripheral : BlePairing
{
public:
    blePairingPeripheral(TX_Function tx_func, RX_Function rx_func, bleControl* ble):
        BlePairing(tx_func, rx_func, ble){}
    bool startPairing();
    void eventPasscodeInputRequested();
    AUTHENTICATION_RESULT loop();
private:
    typedef enum
    {
        WAITING_FOR_START,
        WAITING_FOR_PINCODE,
        PINCODE_RECEIVED,
        PINCODE_SENT
    }AUTHENTICATION_STATE;
    bool setRemoteBleAddress(byte *address, byte length);
    bool getPinCode(uint32_t& pinCode);
    AUTHENTICATION_STATE _state=WAITING_FOR_START;
    uint32_t pincode=0;
    bool bleRequestsPin=false;
};

#endif // BLEPAIRINGPERIPHERAL_H
