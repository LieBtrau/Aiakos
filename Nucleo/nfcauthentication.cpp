#include "nfcauthentication.h"
extern NfcAdapter nfc;
extern NtagSramAdapter ntagAdapter;

static int RNG(uint8_t *dest, unsigned size);
byte writePin;
byte readPin;
extern uint32_t ulStartTime;
extern uint32_t ulStartTime2;

//bool getTagData(nfcAuthentication::ss state);

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
    _nfcAdapter->begin();
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
        byte nfcid[7];
        RNG(nfcid,7);
        cryptop.setNFCIDi(nfcid,7);
    }
    cryptop.generateAsymmetricKey(&RNG);
}

bool nfcAuthentication::loop()
{
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
    static ss state=WAITING_FOR_TAG;
    NdefMessage message;
    byte payload[64];

    if(!_nfcAdapter->tagPresent()){
        state=WAITING_FOR_TAG;
        return false;
    }
    switch(state)
    {
    case WAITING_FOR_TAG:
        if(_nfcAdapter->tagPresent()){
            state=READING_PUBLIC_KEY;
        }
        break;
    case READING_PUBLIC_KEY:
        if(getTagData(state))
        {
            message = NdefMessage();
            payload[0]=NfcSec01::QB;
            cryptop.getPublicKey(payload+1);
            message.addUnknownRecord(payload,1+cryptop.getPublicKeySize());
            digitalWrite(writePin, HIGH);
            if(_nfcAdapter->write(message))
            {
                state=READING_NONCE;
            }
        }
        break;
    case READING_NONCE:
        if(getTagData(state))
        {
            message = NdefMessage();
            payload[0]=NfcSec01::NB;
            cryptop.getLocalNonce(payload+1);
            message.addUnknownRecord(payload,1+cryptop.getNonceSize());
            digitalWrite(writePin, HIGH);
            if(_nfcAdapter->write(message))
            {
                state=WAITING_FOR_MAC_TAG;
            }
        }
        break;
    case WAITING_FOR_MAC_TAG:
        if(getTagData(state))
        {
            message = NdefMessage();
            payload[0]=NfcSec01::MAC_TAG_B;
            cryptop.generateKeyConfirmationTag(payload+1);
            message.addUnknownRecord(payload,1+cryptop.getMacTagSize());
            digitalWrite(writePin, HIGH);
            if(_nfcAdapter->write(message))
            {
                state=WAITING_FOR_TAG;
                digitalWrite(writePin, LOW);
                return true;
            }
        }
        break;
    default:
        state=WAITING_FOR_TAG;
        break;
    }
    digitalWrite(writePin, LOW);
    return false;
}

bool nfcAuthentication::tagLoop(){
    static ss state=WAITING_FOR_READER;
    NdefMessage message;
    byte payload[64];

    if((!_ntagAdapter->readerPresent()))// || (!ntagAdapter.rfReadingDone()))
    {
        state=WAITING_FOR_READER;
    }
    switch(state)
    {
    case WAITING_FOR_READER:
        if(_ntagAdapter->readerPresent())// && ntagAdapter.rfReadingDone())
        {
            message = NdefMessage();
            payload[0]=NfcSec01::QA;
            cryptop.getPublicKey(payload+1);
            message.addUnknownRecord(payload,1+cryptop.getPublicKeySize());
            digitalWrite(writePin, HIGH);
            if(_ntagAdapter->write(message))
            {
                state=READING_PUBLIC_KEY;
            }
        }
        break;
    case READING_PUBLIC_KEY:
        if(getTagData(state))
        {
            message = NdefMessage();
            payload[0]=NfcSec01::NA;
            cryptop.getLocalNonce(payload+1);
            message.addUnknownRecord(payload,1+cryptop.getNonceSize());
            digitalWrite(writePin, HIGH);
            if(_ntagAdapter->write(message))
            {
                state=READING_NONCE;
            }
        }
        break;
    case READING_NONCE:
        if(getTagData(state))
        {
            message = NdefMessage();
            payload[0]=NfcSec01::MAC_TAG_A;
            cryptop.generateKeyConfirmationTag(payload+1);
            message.addUnknownRecord(payload,1+cryptop.getMacTagSize());
            digitalWrite(writePin, HIGH);
            if(_ntagAdapter->write(message))
            {
                state=WAITING_FOR_MAC_TAG;
                digitalWrite(writePin, LOW);
                return true;
            }
        }
        break;
    case WAITING_FOR_MAC_TAG:
        if(getTagData(state))
        {
            state=WAITING_FOR_TAG;
            return true;
        }
        break;
    }
    digitalWrite(writePin, LOW);
    return false;
}
//#endif

bool nfcAuthentication::getTagData(ss state)
{
    //Check if tag has data
    static unsigned long waitStart=millis();
    if(millis()<waitStart+330)
    {
        //Avoid reading tag too frequently which may inhibit I²C access to the tag
        return false;
    }
    waitStart=millis();
    digitalWrite(readPin, HIGH);
    NfcTag nf = _bIsDongle ? _ntagAdapter->read() : _nfcAdapter->read();//takes 101ms on Arduino Due
    digitalWrite(readPin, LOW);
    if(!nf.hasNdefMessage()){
        return true;
    }
    NdefMessage nfm=nf.getNdefMessage();
    nfm.print();
    NdefRecord ndf=nfm.getRecord(0);
    byte dat[ndf.getPayloadLength()];
    ndf.getPayload(dat);

    return false;
        switch(state){
        case nfcAuthentication::READING_PUBLIC_KEY:
            if( (dat[0]!=NfcSec01::QA && !_bIsDongle) || (dat[0]!=NfcSec01::QB && _bIsDongle) )
            {
                return false;
            }
            //Package contents:
            //  MSG_ID | QA or MSG_ID | QB
            cryptop.setRemotePublicKey(dat+1);
            return true;
        case nfcAuthentication::READING_NONCE:
            if( (dat[0]!=NfcSec01::NA && !_bIsDongle) || (dat[0]!=NfcSec01::NB && _bIsDongle) )
            {
                return false;
            }
            //Package contents:
            //  MSG_ID | NA or MSG_ID | NB
            cryptop.generateRandomNonce(&RNG);
            byte remoteUid[nf.getUidLength()];
            nf.getUid(remoteUid,nf.getUidLength());
            return cryptop.calcMasterKeySSE(dat+1, remoteUid,nf.getUidLength());
            //return true;
        case nfcAuthentication::WAITING_FOR_MAC_TAG:
            if( (dat[0]!=NfcSec01::MAC_TAG_A && !_bIsDongle) || (dat[0]!=NfcSec01::MAC_TAG_B && _bIsDongle) )
            {
                return false;
            }
            //Package contents:
            // MSG_ID | MAC_TAG_A or MSG_ID | MAC_TAG_B
            return cryptop.checkKeyConfirmation(dat+1);
        default:
            return false;
        }
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
