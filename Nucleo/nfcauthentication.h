#ifndef NFCAUTHENTICATION_H
#define NFCAUTHENTICATION_H

#include "Arduino.h"
#include "crypto.h"
#include "nfccomm.h"

class nfcAuthentication: public NfcComm
{
public:
    nfcAuthentication(NfcAdapter* nfca);
    nfcAuthentication(NtagSramAdapter* nfca);
    void begin();
    bool loop();
    enum ss{
        WAITING_FOR_PEER,
        READING_PUBLIC_KEY,
        READING_NONCE,
        WAITING_FOR_MAC_TAG,
    };
private:
    static const word PAIRING_TIMEOUT=5000;
    bool readerLoop();
    bool tagLoop();
    bool getTagData();
    NfcSec01 cryptop;
    ss _state;
    unsigned long _pairingStartTime;
};

#endif // NFCAUTHENTICATION_H
