#ifndef RN4020_H
#define RN4020_H

#include "include_me.h"

class rn4020
{
public:
    typedef struct{
        char address[12];
        int addressType;
        char friendlyName[20];
        int rssi;
    }tokenInfo;
    rn4020(PinName pinTX, PinName pinRX, PinName pinRTS, PinName pinCTS);
    bool rebootModule();
    bool setEchoOn(bool bOn);
    bool startScanningForDevices();
    bool getFirstFoundToken(tokenInfo* ti, int iTimeOut_ms);
    bool stopScanningForDevices();
    bool findDevice(uint8_t* address);
    bool isPebbleBee(tokenInfo *ti);
    bool connect(tokenInfo* ti);
    bool unboundPeripherals();
private:
    MTSSerialFlowControl _uart1;
    void sendCommand(const char* cmd);
    bool getResponse(char* resp, int iTimeOut_ms);
    bool checkResponseOk();
};

#endif // RN4020_H
