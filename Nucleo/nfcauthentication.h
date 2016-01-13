#ifndef NFCAUTHENTICATION_H
#define NFCAUTHENTICATION_H

#include "Arduino.h"
#include "NfcTag.h"
#include "crypto.h"
#include "ntagsramadapter.h"
#include "NfcAdapter.h"
#include <PN532_SPI.h>
#include <PN532.h>


class nfcAuthentication
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
        CHECK_MAC_TAG_A,
    };
private:
    static const word PAIRING_TIMEOUT=6000;
    bool readerLoop();
    bool tagLoop();
    bool tagHasData(NfcTag tag);
    bool getTagData();
    NfcSec01 cryptop;
    NfcAdapter* _nfcAdapter;
    NtagSramAdapter* _ntagAdapter;
    bool _bIsDongle;
    ss _state;
    unsigned long pairingStartTime;

};

#endif // NFCAUTHENTICATION_H
