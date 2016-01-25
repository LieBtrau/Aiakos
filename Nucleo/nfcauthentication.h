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
    };
private:
    static const word PAIRING_TIMEOUT=5000;
    bool readerLoop();
    bool tagLoop();
    bool tagHasData(NfcTag tag);
    bool getTagData();
    bool write(byte* data, byte length);
    bool read();
    bool longRfBusyPulseFound();
    NfcSec01 cryptop;
    NfcAdapter* _nfcAdapter;
    NtagSramAdapter* _ntagAdapter;
    bool _bIsDongle;
    ss _state;
    unsigned long _pairingStartTime;
    unsigned long _backoffTime;
    byte _remoteIdLength;
    byte _remoteId[10];
    byte _payload[64];
    byte _payloadLength;
};

#endif // NFCAUTHENTICATION_H
