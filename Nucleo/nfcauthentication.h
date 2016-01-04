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
    nfcAuthentication(bool bIsDongle, byte ss_pin, byte fd_pin);
    void begin();
    void readerLoop();
private:
    typedef enum{
        WAITING_FOR_TAG,
        READER_READING_PUBLIC_KEY,
        READER_SENDING_PUBLIC_KEY,
        READER_WAITING_FOR_MAC_TAG_A
    } ss;
    bool parseTagData(ss state, NfcTag tag);
    NfcSec01 cryptop;
    PN532_SPI pn532spi;
    NfcAdapter nfc;
    Ntag ntag;
    NtagSramAdapter ntagAdapter;
    bool _bIsDongle;
};

#endif // NFCAUTHENTICATION_H
