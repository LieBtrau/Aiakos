#ifndef BLEPAIRING_H
#define BLEPAIRING_H

#include <RHReliableDatagram.h> //for wired comm

class BlePairing
{
public:
    BlePairing(RHReliableDatagram* mgrSer, byte peer);
    bool setRemoteBleAddress(byte* address);
    bool getRemoteBleAddress(byte **address);
    bool setPinCode(uint32_t pinCode);
    bool getPinCode(uint32_t& pinCode);
private:
    RHReliableDatagram* pmgrSer;
    bool writeDataSer(byte* data, byte length);
    bool readDataSer(byte** data, byte& length);
    byte peerAddress;
};

#endif // BLEPAIRING_H
