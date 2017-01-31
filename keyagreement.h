#ifndef KEYAGREEMENT_H
#define KEYAGREEMENT_H
#include <crypto.h>
#include <RHReliableDatagram.h>

class Keyagreement
{
public:
    typedef enum{
        START,
        SEND_ID,
        GET_ID,
        SEND_PUBKEY,
        GET_PUBKEY,
        SEND_NONCE,
        GET_NONCE
    } STATE;
    Keyagreement(bool isInitiator);
    void init();
    bool runKeyAgreement(RHReliableDatagram &datagram, byte peerAddress);
private:
    typedef enum{
        NFCID,
        NONCE,
        RESP_NFCID = NFCID ^ 0x80,
        RESP_NONCE = NONCE ^ 0x80
    } COMM_ID;
    bool waitFor(COMM_ID commid, byte* data, byte& length, unsigned long ulTimeout);
    bool send(COMM_ID commid, byte* data, byte length);
    NfcSec01 _sec;
    RHReliableDatagram* _datagram;
    byte _peer;
    byte _data[64];
    bool _isInitiator;
};

#endif // KEYAGREEMENT_H
