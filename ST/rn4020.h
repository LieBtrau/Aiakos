#ifndef RN4020_H
#define RN4020_H

#include "include_me.h"

class rn4020
{
public:
    rn4020(PinName pinTX, PinName pinRX, PinName pinRTS, PinName pinCTS);
    bool rebootModule();
    bool setEchoOn(bool bOn);
    bool startScanningForDevices();
    bool getFirstFoundToken(char* foundToken, int& RSSI, int iTimeOut_ms);
    bool stopScanningForDevices();
    bool findDevice(uint8_t* address);
private:
    MTSSerialFlowControl _uart1;
    void sendCommand(const char* cmd);
    bool getResponse(char* resp, int iTimeOut_ms);
    bool checkResponseOk();
};

#endif // RN4020_H
