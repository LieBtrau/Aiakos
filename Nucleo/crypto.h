#ifndef CRYPTO_H
#define CRYPTO_H
#include "Arduino.h"
#include <uECC.h>
#include "cmac.h"

class Crypto
{
public:
    Crypto();
    void getLocalMacTag(const byte* pRemotePublicKey, const byte* pRemoteNFCID3, byte* localMacTag, bool bIsInitiator);
    byte* getLocalNonce(bool bGenerateNew);
    void setNFCIDi(byte* nfcid3i);
    bool setLocalKey(char *pLocalPrivateKey, char *pLocalPublicKey);
    bool calcMasterKeySSE(const byte* pRemotePublicKey, const byte* pRemoteNonce, const byte* pRemoteNFCID3, bool bIsInitiator);
    bool checkKeyConfirmation(const byte* pRemoteMacTag, const byte* pLocalMacTag);
    void testEcc();
    bool testMasterKeySse();
    void testCmac();
    static const byte NFCID_SIZE=10;
private:
    void printBuffer(const char *name, const byte* buf, byte len);
    byte _localPrivateKey[uECC_BYTES+1];
    byte _localPublicKey[uECC_BYTES*2+1];
    byte _localNonce[uECC_BYTES];
    byte _localNFCID3[NFCID_SIZE];
    byte _MKsse[BLOCK_SIZE];

};

#endif // CRYPTO_H
