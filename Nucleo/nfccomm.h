#ifndef NFCCOMM_H
#define NFCCOMM_H

#include "Arduino.h"
#include "NfcTag.h"
#include "ntagsramadapter.h"
#include "NfcAdapter.h"
#include <PN532_SPI.h>
#include <PN532.h>

class NfcComm
{
public:
    NfcComm(byte wPin, byte rPin);
    NfcComm(byte wPin, byte rPin, NfcAdapter* nfca);
    void setup();
protected:
    bool write(byte* data, byte length);
    bool read();
    bool tagPresent(bool& bPresent);
    bool readerHasTagRead();
    byte _remoteIdLength;
    byte _remoteId[10];
    byte _payload[64];
    byte _payloadLength;
    NtagSramAdapter* _ntagAdapter;
    bool _bIsDongle;
private:
    NfcAdapter* _nfcAdapter;
    unsigned long _backoffTime;
};

#endif // NFCCOMM_H
