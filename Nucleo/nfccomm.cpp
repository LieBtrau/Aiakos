#include "nfccomm.h"
#include "RHCRC.h"

byte writePin;
byte readPin;

NfcComm::NfcComm(byte wPin, byte rPin)
{
    writePin=wPin;
    readPin=rPin;
}

NfcComm::NfcComm(byte wPin, byte rPin, NfcAdapter *nfca):
    _nfcAdapter(nfca)
{
    writePin=wPin;
    readPin=rPin;
}

void NfcComm::setup()
{
    pinMode(writePin, OUTPUT);
    pinMode(readPin, OUTPUT);
    if (_bIsDongle)
    {
        _ntagAdapter->begin();
    }
    else
    {
        _nfcAdapter->begin();
        _backoffTime=millis();
    }
}

//Return false when timeout has not yet passed.
bool NfcComm::tagPresent(bool& bPresent)
{
    if (millis() < _backoffTime + 150)
    {
        //Avoid reading tag too frequently which may inhibit I²C access to the tag
        return false;
    }
    bPresent=_nfcAdapter->tagPresent();
    _backoffTime=millis();
    return true;
}

//The tag can detect that the reader has read the tag by monitoring the FD-pin.
bool NfcComm::readerHasTagRead()
{
    static byte state=0;
    static unsigned long pulseStart;

    switch(state)
    {
    case 0:
        if(_ntagAdapter->rfBusy())
        {
            state=1;
            pulseStart=millis();
        }
        break;
    case 1:
        if(!_ntagAdapter->rfBusy())
        {
            //end of pulse detected
            if(millis()>pulseStart+50)
            {
                //long pulse detected
                state=0;
                return true;
            }else
            {
                //too short pulse detected
                state=0;
            }
        }
        break;
    default:
        state=0;
        break;
    }
    return false;
}

bool NfcComm::read()
{
    bool bPresent;
    uint16_t crc = 0xffff;
    digitalWrite(readPin, HIGH);
    if (!_bIsDongle && ((!tagPresent(bPresent)) || !bPresent))
    {
        return false;
    }
    NfcTag nf = _bIsDongle ? _ntagAdapter->read(200) : _nfcAdapter->read();//takes 101ms on Arduino Due
    digitalWrite(readPin, LOW);
    _remoteIdLength = nf.getUidLength();
    nf.getUid(_remoteId, _remoteIdLength);
    if (!nf.hasNdefMessage()) {
        return false;
    }
    NdefMessage nfm = nf.getNdefMessage();
    if (nfm.getRecordCount() == 0)
    {
        return false;
    }
    NdefRecord ndf = nfm.getRecord(0);
    _payloadLength = ndf.getPayloadLength();
    ndf.getPayload(_payload);
    for (byte i = 0; i < _payloadLength - 2; i++)
    {
        crc = RHcrc_ccitt_update(crc, _payload[i]);
    }
    if (lowByte(crc) != _payload[_payloadLength - 2] || highByte(crc) != _payload[_payloadLength - 1])
    {
        Serial.println("Wrong CRC");
        return false;
    }
    _payloadLength -= 2;
    return true;
}

bool NfcComm::write(byte* data, byte length)
{
    bool bResult;
    NdefMessage message = NdefMessage();
    uint16_t crc = 0xffff;
    for (byte i = 0; i < length; i++)
    {
        crc = RHcrc_ccitt_update(crc, data[i]);
    }
    data[length] = lowByte(crc);
    data[length + 1] = highByte(crc);
    message.addUnknownRecord(data, length + 2);
    digitalWrite(writePin, HIGH);
    if (_bIsDongle)
    {
        bResult = _ntagAdapter->write(message, 200);
    }
    else
    {
        bResult = _nfcAdapter->write(message);
    }
    digitalWrite(writePin, LOW);
    return bResult;
}
