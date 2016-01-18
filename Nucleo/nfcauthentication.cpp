#include "nfcauthentication.h"
#include "RHCRC.h"

extern NfcAdapter nfc;
extern NtagSramAdapter ntagAdapter;

static int RNG(uint8_t *dest, unsigned size);
byte writePin;
byte readPin;
extern uint32_t ulStartTime;
extern uint32_t ulStartTime2;

void print(nfcAuthentication::ss state, byte data)
{
    switch (state) {
    case nfcAuthentication::WAITING_FOR_PEER:
        Serial.print("WAITING_FOR_PEER");
        break;
    case nfcAuthentication::READING_PUBLIC_KEY:
        Serial.print("READING_PUBLIC_KEY");
        break;
    case nfcAuthentication::READING_NONCE:
        Serial.print("READING_NONCE");
        break;
    case nfcAuthentication::WAITING_FOR_MAC_TAG:
        Serial.print("WAITING_FOR_MAC_TAG");
        break;
    default:
        Serial.print("unknown state");
        break;
    }
    Serial.print("\t");
    switch(data)
    {
    case NfcSec01::QA:
        Serial.print("QA");
        break;
    case NfcSec01::QB:
        Serial.print("QB");
        break;
    case NfcSec01::NA:
        Serial.print("NA");
        break;
    case NfcSec01::NB:
        Serial.print("NB");
        break;
    case NfcSec01::MAC_TAG_A:
        Serial.print("MAC_TAG_A");
        break;
    case NfcSec01::MAC_TAG_B:
        Serial.print("MAC_TAG_B");
        break;
    default:
        Serial.print("unknown data");
        break;
    }
    Serial.println();
}

void print(const byte* array, byte length)
{
    for(byte i=0;i<length;i++)
    {
        Serial.print(array[i], HEX);
        Serial.print(" ");
        if((i+1)%16==0)
        {
            Serial.println();
        }
    }
    Serial.println();
}

nfcAuthentication::nfcAuthentication(NtagSramAdapter* nfca):
    _ntagAdapter(nfca)
{
    _bIsDongle=true;
    cryptop.setInitiator(_bIsDongle);
    writePin =  3;
    readPin =  4;
}

nfcAuthentication::nfcAuthentication(NfcAdapter* nfca):
    _nfcAdapter(nfca)
{
    _bIsDongle=false;
    cryptop.setInitiator(_bIsDongle);
    writePin =  18;
    readPin =  19;
}

void nfcAuthentication::begin()
{
    pinMode(writePin, OUTPUT);
    pinMode(readPin, OUTPUT);
    if(_bIsDongle)
    {
        _ntagAdapter->begin();
        byte data[_ntagAdapter->getUidLength()];
        _ntagAdapter->getUid(data, _ntagAdapter->getUidLength());
        cryptop.setNFCIDi(data,_ntagAdapter->getUidLength());
    }else
    {
        _nfcAdapter->begin();
    }
    cryptop.generateAsymmetricKey(&RNG);
}

bool nfcAuthentication::loop()
{
    if(_state==WAITING_FOR_PEER)
    {
        pairingStartTime=millis();
    }
    if(millis()>pairingStartTime+ PAIRING_TIMEOUT)
    {
        _state=WAITING_FOR_PEER;
        pairingStartTime=millis();
    }
    if(_bIsDongle)
    {
        return tagLoop();
    }else
    {
        return readerLoop();
    }
}

bool nfcAuthentication::readerLoop()
{
    byte payload[64];

    if(!_nfcAdapter->tagPresent()){
        _state=WAITING_FOR_PEER;
        return false;
    }
    switch(_state)
    {
    case WAITING_FOR_PEER:
        if(_nfcAdapter->tagPresent()){
            _state=READING_PUBLIC_KEY;
        }
        break;
    case READING_PUBLIC_KEY:
        if(getTagData())
        {
            payload[0]=NfcSec01::QB;
            cryptop.getPublicKey(payload+1);
            if(write(payload,1+cryptop.getPublicKeySize()))
            {
                Serial.println("QB written");
                _state=READING_NONCE;
            }
        }
        break;
    case READING_NONCE:
        if(getTagData())
        {
            payload[0]=NfcSec01::NB;
            cryptop.getLocalNonce(payload+1);
            if(write(payload,1+cryptop.getNonceSize()))
            {
                Serial.println("NB written");
                _state=WAITING_FOR_MAC_TAG;
            }
        }
        break;
    case WAITING_FOR_MAC_TAG:
        if(getTagData())
        {
            payload[0]=NfcSec01::MAC_TAG_B;
            cryptop.generateKeyConfirmationTag(payload+1);
            if(write(payload,1+cryptop.getMacTagSize()))
            {
                Serial.println("MacTagB written");
                _state=WAITING_FOR_PEER;
                return true;
            }
        }
        break;
    default:
        _state=WAITING_FOR_PEER;
        break;
    }
    digitalWrite(writePin, LOW);
    return false;
}

bool nfcAuthentication::tagLoop(){
    byte payload[64];

    if((!_ntagAdapter->readerPresent()))// || (!ntagAdapter.rfReadingDone()))
    {
        _state=WAITING_FOR_PEER;
    }
    switch(_state)
    {
    case WAITING_FOR_PEER:
        if(_ntagAdapter->readerPresent())// && ntagAdapter.rfReadingDone())
        {
            payload[0]=NfcSec01::QA;
            cryptop.getPublicKey(payload+1);
            if(write(payload,1+cryptop.getPublicKeySize()))
            {
                Serial.println("QA written");
                _state=READING_PUBLIC_KEY;
            }
        }
        break;
    case READING_PUBLIC_KEY:
        if(getTagData())
        {
            payload[0]=NfcSec01::NA;
            cryptop.getLocalNonce(payload+1);
            if(write(payload,1+cryptop.getNonceSize()))
            {
                Serial.println("NA written");
                _state=READING_NONCE;
            }
        }
        break;
    case READING_NONCE:
        if(getTagData())
        {
            payload[0]=NfcSec01::MAC_TAG_A;
            cryptop.generateKeyConfirmationTag(payload+1);
            if(write(payload,1+cryptop.getMacTagSize()))
            {
                Serial.println("MacTagA written");
                _state=WAITING_FOR_MAC_TAG;
            }
        }
        break;
    case WAITING_FOR_MAC_TAG:
        if(getTagData())
        {
            _state=WAITING_FOR_PEER;
            return true;
        }
        break;
    default:
        _state=WAITING_FOR_PEER;
        break;
    }
    return false;
}
//#endif

bool nfcAuthentication::getTagData()
{
    //Check if tag has data
    static unsigned long waitStart=millis();
    if(millis()<waitStart+ (_bIsDongle ? 660 : 330))
    {
        //Avoid reading tag too frequently which may inhibit I²C access to the tag
        return false;
    }
    waitStart=millis();
     NfcTag nf = read();
    byte remoteUid[nf.getUidLength()];
    if(!nf.hasNdefMessage()){
        return false;
    }
    NdefMessage nfm=nf.getNdefMessage();
    NdefRecord ndf=nfm.getRecord(0);
    byte dat[ndf.getPayloadLength()];
    ndf.getPayload(dat);
    print(_state, dat[0]);
    switch(_state){
    case READING_PUBLIC_KEY:
        if(!_bIsDongle)
        {
            if(dat[0]!=NfcSec01::QA)
            {
                return false;
            }
        }
        else
        {
            if(dat[0]==NfcSec01::QA)
            {
                return false;
            }
            if(dat[0]!=NfcSec01::QB)
            {
                _state=WAITING_FOR_PEER;
                return false;
            }
        }
        //Package contents:
        //  MSG_ID | QA or MSG_ID | QB
        cryptop.setRemotePublicKey(dat+1);
        cryptop.generateRandomNonce(&RNG);
        return true;
    case READING_NONCE:
        if(!_bIsDongle)
        {
            if(dat[0]==NfcSec01::QB)
            {
                return false;
            }
            if(dat[0]!=NfcSec01::NA)
            {
                _state=READING_PUBLIC_KEY;
                return false;
            }
        }
        else
        {
            if(dat[0]==NfcSec01::NA)
            {
                return false;
            }
            if(dat[0]==NfcSec01::QB)
            {
                _state=READING_PUBLIC_KEY;
                return false;
            }
            if(dat[0]!=NfcSec01::NB)
            {
                _state=WAITING_FOR_PEER;
                return false;
            }
        }
        //Package contents:
        //  MSG_ID | NA or MSG_ID | NB
        nf.getUid(remoteUid,nf.getUidLength());
        //Reader and tag use the same NFCID because the reader doesn't have an NFCID.
        cryptop.setNFCIDi(remoteUid,nf.getUidLength());
        return cryptop.calcMasterKeySSE(dat+1, remoteUid,nf.getUidLength());
        break;
    case WAITING_FOR_MAC_TAG:
        if(!_bIsDongle)
        {
            if(dat[0]==NfcSec01::NB)
            {
                return false;
            }
            if(dat[0]==NfcSec01::NA)
            {
                _state=READING_NONCE;
                return false;
            }
            if(dat[0]!=NfcSec01::MAC_TAG_A)
            {
                _state=READING_PUBLIC_KEY;
                return false;
            }
        }
        else
        {
            if(dat[0]==NfcSec01::MAC_TAG_A)
            {
                return false;
            }
            if(dat[0]==NfcSec01::NB)
            {
                _state=READING_NONCE;
                return false;
            }
            if(dat[0]!=NfcSec01::MAC_TAG_B)
            {
                _state=WAITING_FOR_PEER;
                return false;
            }
        }
        //Package contents:
        // MSG_ID | MAC_TAG_A or MSG_ID | MAC_TAG_B
        return cryptop.checkKeyConfirmation(dat+1);
    default:
        return false;
    }
    return false;
}

NfcTag nfcAuthentication::read()
{
    uint16_t crc = 0xffff;
    digitalWrite(readPin, HIGH);
    NfcTag nf = _bIsDongle ? _ntagAdapter->read() : _nfcAdapter->read();//takes 101ms on Arduino Due
    digitalWrite(readPin, LOW);
    byte uidLength=nf.getUidLength();
    byte remoteUid[uidLength];
    nf.getUid(remoteUid,uidLength);
    if(!nf.hasNdefMessage()){
        return NfcTag(remoteUid, uidLength,"NTAG");
    }
    NdefMessage nfm=nf.getNdefMessage();
    NdefRecord ndf=nfm.getRecord(0);
    byte length=ndf.getPayloadLength();
    byte data[length];
    ndf.getPayload(data);
    for(byte i=0;i<length-2;i++)
    {
        crc = RHcrc_ccitt_update(crc, data[i]);
    }
    if(lowByte(crc)!= data[length-2] || highByte(crc)!=data[length-1])
    {
        Serial.println("Wrong CRC");
        return NfcTag(remoteUid, uidLength,"NTAG");
    }
    NdefMessage message = NdefMessage();
    message.addUnknownRecord(data,length-2);
    return NfcTag(remoteUid,uidLength,"NTAG", message);
}

bool nfcAuthentication::write(byte* data, byte length)
{
    bool bResult;
    NdefMessage message = NdefMessage();
    uint16_t crc = 0xffff;
    for(byte i=0;i<length;i++)
    {
        crc = RHcrc_ccitt_update(crc, data[i]);
    }
    data[length]=lowByte(crc);
    data[length+1]=highByte(crc);
    message.addUnknownRecord(data,length+2);
    digitalWrite(writePin, HIGH);
    if(_bIsDongle)
    {
        bResult=_ntagAdapter->write(message);
    }
    else
    {
        bResult= _nfcAdapter->write(message);
    }
    digitalWrite(writePin, LOW);
    return bResult;
}

//TODO: replace by safe external RNG
static int RNG(uint8_t *dest, unsigned size) {
    // Use the least-significant bits from the ADC for an unconnected pin (or connected to a source of
    // random noise). This can take a long time to generate random data if the result of analogRead(0)
    // doesn't change very frequently.
    while (size) {
        uint8_t val = 0;
        for (unsigned i = 0; i < 8; ++i) {
            int init = analogRead(0);
            int count = 0;
            while (analogRead(0) == init) {
                ++count;
            }

            if (count == 0) {
                val = (val << 1) | (init & 0x01);
            } else {
                val = (val << 1) | (count & 0x01);
            }
        }
        *dest = val;
        ++dest;
        --size;
    }
    // NOTE: it would be a good idea to hash the resulting random data using SHA-256 or similar.
    return 1;
}
