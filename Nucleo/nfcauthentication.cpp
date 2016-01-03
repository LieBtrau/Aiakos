#include "nfcauthentication.h"

nfcAuthentication::nfcAuthentication(bool bIsInitiator, byte ss_pin, byte fd_pin):
    cryptop(bIsInitiator),
    pn532spi(SPI, ss_pin),
    ntagAdapter(&ntag),
    nfc(pn532spi),
    ntag(Ntag::NTAG_I2C_1K, fd_pin),
    _bIsInitiator(bIsInitiator)
{
}

void nfcAuthentication::begin()
{
    if(_bIsInitiator)
    {
        pinMode(3, OUTPUT);
        ntagAdapter.begin();
        //tagLoop(true);
    }else
    {
        pinMode(18, OUTPUT);
        nfc.begin();
    }
}

void nfcAuthentication::loop()
{
    static unsigned long waitStart=millis();
    static ss state=WAITING_FOR_TAG;
    NfcTag tag;

    switch(state)
    {
    case WAITING_FOR_TAG:
        if(nfc.tagPresent()){
            state=READING_PUBLIC_KEY;
        }
        break;
    case READING_PUBLIC_KEY:
        if(!nfc.tagPresent()){
            state=WAITING_FOR_TAG;
        }
        if(millis()<waitStart+330)
        {
            //Avoid reading to frequently which may inhibit I²C access to the tag
            return;
        }
        waitStart=millis();
        digitalWrite(18, HIGH);
        tag = nfc.read();//takes 101ms on Arduino Due
        if(!tag.hasNdefMessage()){
            digitalWrite(18, LOW);
            return;
        }
        if(parseTagData(READING_PUBLIC_KEY, tag))
        {
            state=SENDING_PUBLIC_KEY;
        }
        break;
    case SENDING_PUBLIC_KEY:
        break;
    }

    //Full cycle (tag read by RF, tag written by RF, tag read by I²C, tag written by I²C) takes less than 330ms.
    //    if(bitRead(dat[0],0)){
    //        NdefMessage message = NdefMessage();
    //        data[0]=dat[0]+1;
    //        message.addUnknownRecord(data,sizeof(data));
    //        if(nfc.write(message)){
    //            Serial.println("Reader has written message to tag.");
    //        }else{
    //            Serial.println("Failed to write to tag");
    //        }
    //    }
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
    case READING_PUBLIC_KEY:
        if(dat[0]!=NfcSec01::QA_AND_NA)
        {
            return false;
        }
        //Package contents:
        //  MSG_ID | QA | NA
        byte uid[NfcSec01::NFCID_SIZE];
        memset(uid,0,NfcSec01::NFCID_SIZE);
        tag.getUid(uid,tag.getUidLength());
        return cryptop.calcMasterKeySSE(dat+1,dat+1+2*NfcSec01::_192BIT_, uid);
    default:
        return false;
    }
}
