#include "keyagreement.h"

#define DEBUG
static void printBuffer(const char* name, const byte* buf, byte len);
static int RNG(uint8_t *dest, unsigned size);

Keyagreement::Keyagreement(bool isInitiator):
    _isInitiator(isInitiator),
    _sec(isInitiator)
{
}

//generation of keys should be done only once and then stored in EEPROM
void Keyagreement::init()
{
    //Key
    _sec.generateAsymmetricKey(&RNG);
    //NFCID3
    RNG(_data, NfcSec01::NFCID_SIZE); //Should be read from the NFC-tag
    _sec.setNFCIDi(_data, NfcSec01::NFCID_SIZE);
}


bool Keyagreement::runKeyAgreement(RHReliableDatagram& datagram, byte peerAddress)
{
    _datagram=&datagram;
    _peer=peerAddress;
    byte rnfcid[NfcSec01::NFCID_SIZE];
    byte length;
    if(_isInitiator)
    {
        //send ID + public key
        length=NfcSec01::NFCID_SIZE;
        _sec.getNFCIDi(_data, length);
        _sec.getPublicKey(_data + NfcSec01::NFCID_SIZE);
        if(!send(NFCID, _data, NfcSec01::NFCID_SIZE + _sec.getPublicKeySize()))
        {
            return false;
        }
        //wait for ID & public key of peer
        if(!waitFor(RESP_NFCID, _data, length, 3000))
        {
            return false;
        }
        memcpy(rnfcid, _data, NfcSec01::NFCID_SIZE);
        _sec.setRemotePublicKey(_data+NfcSec01::NFCID_SIZE);
        //Send Nonce
        _sec.generateRandomNonce(&RNG);
        _sec.getLocalNonce(_data);
        if(!send(NONCE, _data, _sec.getNonceSize()))
        {
            return false;
        }
        //wait for nonce of peer
        if(!waitFor(RESP_NONCE, _data, length, 3000))
        {
            return false;
        }
        if(!_sec.calcMasterKeySSE(_data,rnfcid,NfcSec01::NFCID_SIZE))
        {
            return false;
        }
#ifdef DEBUG
        _sec.getMasterKey(_data);
        printBuffer("MKsseA", _data, _sec.getMasterKeySize());
#endif
    }
    else{

    }
}


bool testMasterKeySse() {
    //  //Generate master key on unit A:
    //  unitA.setRemotePublicKey(publicB);
    //  if (!unitA.calcMasterKeySSE(nonceB, NFCID3_B, NfcSec01::NFCID_SIZE)) {
    //    Serial.println("Can't calculate master keyA");
    //    return false;
    //  }
    //  unitA.getMasterKey(MKsseA);
    //  printBuffer("MKsseA", MKsseA, unitA.getMasterKeySize());
    
    //  //Generate master key on unit B:
    //  unitB.setRemotePublicKey(publicA);
    //  if (!unitB.calcMasterKeySSE(nonceA, NFCID3_A, NfcSec01::NFCID_SIZE)) {
    //    Serial.println("Can't calculate master keyB");
    //    return false;
    //  }
    //  unitB.getMasterKey(MKsseB);
    //  printBuffer("MKsseB", MKsseB, unitB.getMasterKeySize());
    
    //  if(memcmp(MKsseA,MKsseB,unitA.getMasterKeySize())!=0){
    //    Serial.println("Master keys are not equal");
    //    return false;
    //  }
    
    //  //Generate key confirmation tag on unit A = MacTagA
    //  unitA.generateKeyConfirmationTag(macTagA);
    //  printBuffer("macTagA", macTagA, unitA.getMacTagSize());
    
    //  //Unit B checks MacTagA
    //  if (!unitB.checkKeyConfirmation(macTagA)) {
    //    Serial.println("Key confirmation fails");
    //    return false;
    //  }
    
    //  //Generate key confirmation tag on unit B = MacTagB
    //  unitB.generateKeyConfirmationTag(macTagB);
    //  printBuffer("macTagB", macTagB, unitB.getMacTagSize());
    
    //  //Unit A checks MacTagB
    //  if (!unitA.checkKeyConfirmation(macTagB)) {
    //    Serial.println("Key confirmation fails");
    //    return false;
    //  }
    //  Serial.println("Key confirmation successful");
    //  return true;
}

bool Keyagreement::send(COMM_ID commid, byte* data, byte length)
{
    memmove(data+1, data, length);
    data[0]==commid;
    return _datagram->sendtoWait(data, length+1, _peer);
}

bool Keyagreement::waitFor(COMM_ID commid, byte* data, byte& length, unsigned long ulTimeout)
{
    byte peer;
    unsigned long ulStartTime=millis();
    while(millis()<ulStartTime+ulTimeout)
    {
        if (_datagram->available() && _datagram->recvfromAck(data, &length, &peer) && peer==_peer && data[0]==commid)
        {
            memmove(data,data+1, length-1);
            return true;
        }
    }
    return false;
}

void printBuffer(const char* name, const byte* buf, byte len)
{
    Serial.print(name);
    Serial.print(": ");
    for (int i = 0; i < len; i++) {
        Serial.print(buf[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
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
    // NOTE: it would be a good idea to hash the resulting random data using SHA-B56 or similar.
    return 1;
}
