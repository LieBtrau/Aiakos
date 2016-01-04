#include "nfcauthentication.h"

static int RNG(uint8_t *dest, unsigned size);

nfcAuthentication::nfcAuthentication(bool bIsDongle, byte ss_pin, byte fd_pin):
    cryptop(bIsDongle),
    pn532spi(SPI, ss_pin),
    nfc(pn532spi),
    ntag(Ntag::NTAG_I2C_1K, fd_pin),
    ntagAdapter(&ntag),
    _bIsDongle(bIsDongle)
{
}

void nfcAuthentication::begin()
{
    if(_bIsDongle)
    {
        pinMode(3, OUTPUT);
        ntagAdapter.begin();
        byte data[ntagAdapter.getUidLength()];
        ntagAdapter.getUid(data, ntagAdapter.getUidLength());
        cryptop.setNFCIDi(data,ntagAdapter.getUidLength());
    }else
    {
        pinMode(18, OUTPUT);
        nfc.begin();
        cryptop.generateRandomNFCIDi(&RNG);
    }
    cryptop.generateAsymmetricKey(&RNG);
}

void nfcAuthentication::readerLoop()
{
    static unsigned long waitStart=millis();
    static ss state=WAITING_FOR_TAG;
    NfcTag tag;
    NdefMessage message;
    byte payload[64];

    if(!nfc.tagPresent()){
        state=WAITING_FOR_TAG;
        return;
    }
    switch(state)
    {
    case WAITING_FOR_TAG:
        if(nfc.tagPresent()){
            state=READER_READING_PUBLIC_KEY;
        }
        break;
    case READER_READING_PUBLIC_KEY:
        if(millis()<waitStart+330)
        {
            //Avoid reading tag too frequently which may inhibit I²C access to the tag
            return;
        }
        waitStart=millis();
        digitalWrite(18, HIGH);
        tag = nfc.read();//takes 101ms on Arduino Due
        if(!tag.hasNdefMessage()){
            digitalWrite(18, LOW);
            return;
        }
        if(parseTagData(READER_READING_PUBLIC_KEY, tag))
        {
            state=READER_SENDING_PUBLIC_KEY;
        }
        break;
    case READER_SENDING_PUBLIC_KEY:
        message = NdefMessage();
        payload[0]=NfcSec01::QB_AND_NB;
        cryptop.getPublicKey(payload+1);
        cryptop.getLocalNonce(payload+1+cryptop.getPublicKeySize());
        message.addUnknownRecord(payload,1+cryptop.getPublicKeySize()+cryptop.getNonceSize());
        if(nfc.write(message))
        {
            state=READER_WAITING_FOR_MAC_TAG_A;
        }
        break;
    }
    digitalWrite(18, LOW);
}

//void nfcAuthentication::tagLoop(bool bInitialize){
//    byte id=0;
//    if(!bInitialize){
//        if(!ntagAdapter.rfReadingDone()){
//            return;
//        }
//        delay(150);//Give RF-interface time to write the tag.
//        NfcTag nf=ntagAdapter.read();
//        if(!nf.hasNdefMessage()){
//            return;
//        }
//        NdefMessage nfm=nf.getNdefMessage();
//        nfm.print();
//        NdefRecord ndf=nfm.getRecord(0);
//        byte dat[ndf.getPayloadLength()];
//        ndf.getPayload(dat);
//        id=dat[0];
//    }
//    //    if(bitRead(id,0)==0 || bInitialize){
//    //        digitalWrite(3, HIGH);
//    //        data[0]=id+1;
//    //        NdefMessage message = NdefMessage();
//    //        message.addUnknownRecord(data,sizeof(data));
//    //        if(ntagAdapter.write(message)){
//    //            Serial.println("I²C has written message to tag.");
//    //        }
//    //        ntag.releaseI2c();
//    //    }
//    digitalWrite(3, LOW);
//}
////#endif

bool nfcAuthentication::parseTagData(ss state, NfcTag tag)
{
    NdefMessage nfm=tag.getNdefMessage();
    nfm.print();
    NdefRecord ndf=nfm.getRecord(0);
    byte dat[ndf.getPayloadLength()];
    ndf.getPayload(dat);

    switch(state){
    case READER_READING_PUBLIC_KEY:
        if(dat[0]!=NfcSec01::QA_AND_NA)
        {
            return false;
        }
        //Package contents:
        //  MSG_ID | QA | NA
        byte remoteUid[NfcSec01::NFCID_SIZE];
        memset(remoteUid,0,NfcSec01::NFCID_SIZE);
        tag.getUid(remoteUid,tag.getUidLength());
        cryptop.generateRandomNonce(&RNG);
        return cryptop.calcMasterKeySSE(dat+1,dat+1+2*NfcSec01::_192BIT_, remoteUid);
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
