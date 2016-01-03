#ifndef NFCAUTHENTICATION_H
#define NFCAUTHENTICATION_H

#include "Arduino.h"
#include "NfcTag.h"
#include "crypto.h"
#include "ntagsramadapter.h"
#include "NfcAdapter.h"
#include <PN532_SPI.h>
#include <PN532.h>

typedef enum{
    WAITING_FOR_TAG,
    READING_PUBLIC_KEY,
    SENDING_PUBLIC_KEY
} ss;


class nfcAuthentication
{
public:
    nfcAuthentication(bool bInitializer, byte ss_pin, byte fd_pin);
    void begin();
    void loop();
private:
    bool parseTagData(ss state, NfcTag tag);
    PN532_SPI pn532spi;
    NfcAdapter nfc;
    NfcSec01 cryptop;
    Ntag ntag;
    NtagSramAdapter ntagAdapter;
    bool _bIsInitiator;
};

#endif // NFCAUTHENTICATION_H
