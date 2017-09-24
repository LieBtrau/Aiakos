#ifndef BLEPAIRINGPERIPHERAL_H
#define BLEPAIRINGPERIPHERAL_H
#include "blepairing.h"

class blePairingPeripheral : BlePairing
{
public:
    blePairingPeripheral(TX_Function tx_func, RX_Function rx_func, bleControl* ble, byte rfidKeyLength):
        BlePairing(tx_func, rx_func, ble, rfidKeyLength){}
    bool startPairing();
    void eventPasscodeInputRequested();
    void eventBondingBonded();
    bool getRfidKey(byte key[]);
    AUTHENTICATION_RESULT loop();
private:
    typedef enum
    {
        WAITING_FOR_START,
        WAITING_FOR_RFID_KEY,
        WAITING_FOR_PASSCODE,
        PASSCODE_RECEIVED,
        PASSCODE_SENT
    }AUTHENTICATION_STATE;
    AUTHENTICATION_STATE _state=WAITING_FOR_START;
    uint32_t passcode=0;
    bool bleRequestsPass=false;
    bool bondingBonded=false;
};

#endif // BLEPAIRINGPERIPHERAL_H
