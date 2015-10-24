#ifndef CRYPTO_H
#define CRYPTO_H
#include "Arduino.h"
#include <uECC.h>
#include "cmac.h"

class Crypto
{
public:
    Crypto();
    bool setLocalKey(char *pLocalPrivateKey, char *pLocalPublicKey);
    bool calcMasterKeySSE(const byte* pRemotePublicKey, const byte* pRemoteNonce, const byte* pRemoteNFCID3, bool bIsInitiator);
    byte* getNa();
    void setNFCIDi(byte* nfcid3i);
    void testEcc();
    bool testMasterKeySse();
    void testCmac();
    static const byte NFCID_SIZE=10;
private:
    void printBuffer(const char *name, const byte* buf, byte len);
    byte _pLocalPrivateKey[uECC_BYTES+1];
    byte _pLocalPublicKey[uECC_BYTES*2+1];
    byte _localNonce[uECC_BYTES];
    byte _NFCID3i[NFCID_SIZE];
    byte _MKsse[BLOCK_SIZE];

};

#endif // CRYPTO_H
