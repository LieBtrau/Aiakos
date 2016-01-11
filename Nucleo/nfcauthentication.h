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
    typedef enum{
        WAITING_FOR_TAG,
        WAITING_FOR_READER,
        READING_PUBLIC_KEY,
        READING_NONCE,
        WAITING_FOR_MAC_TAG,
        CHECK_MAC_TAG_A,
    } ss;
private:
    bool readerLoop();
    bool tagLoop();
    bool tagHasData(NfcTag tag);
    bool getTagData(ss state);
    NfcSec01 cryptop;
    NfcAdapter* _nfcAdapter;
    NtagSramAdapter* _ntagAdapter;
    bool _bIsDongle;
};

#endif // NFCAUTHENTICATION_H
