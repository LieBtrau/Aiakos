/* Source code for RN4020 can be found on: http://ww1.microchip.com/downloads/en/DeviceDoc/rn4020_pictail__pic18_usb_uart_firmware.zip
 */

#include "rn4020.h"
#include <cstring>

static const char RX_BUFFER_SIZE=64;
char rx_buffer[RX_BUFFER_SIZE];
static const char* REBOOT="Reboot";
static const char* CMD="CMD";
static const char* ECHO_ON="Echo On";
static const char* ECHO_OFF="Echo Off";
static const char* AOK="AOK";
static const char* OUI_PEBBLEBEE="0E0A14"; //not officially registered with IEEE
static const char* CONNPARAM="ConnParam";

rn4020::rn4020(PinName pinTX, PinName pinRX, PinName pinRTS, PinName pinCTS): _uart1(pinTX, pinRX, pinRTS, pinCTS)
{
    _uart1.baud(9600);
}

bool rn4020::rebootModule()
{
    sendCommand("R,1");
    if(!getResponse(rx_buffer, 3000)){
        return false;
    }
    if(strncmp(rx_buffer, REBOOT, strlen(REBOOT))!=0){
        return false;
    }
    if(!getResponse(rx_buffer, 3000)){
        return false;
    }
    if(strncmp(rx_buffer, CMD, strlen(CMD))!=0){
        return false;
    }
    return true;
}

bool rn4020::isPebbleBee(tokenInfo* ti){
    return strncmp(ti->address, OUI_PEBBLEBEE, strlen(OUI_PEBBLEBEE))==0;
}

bool rn4020::setEchoOn(bool bOn)
{
    for(int i=0;i<3;i++){
        sendCommand("+");
        while(getResponse(rx_buffer, 1000)){
            if( (strncmp(rx_buffer, ECHO_ON, strlen(ECHO_ON)) && bOn) || (strncmp(rx_buffer, ECHO_OFF, strlen(ECHO_OFF)) && !bOn) ){
                return true;
            }
        }
    }
    return false;
}

bool rn4020::startScanningForDevices()
{
    sendCommand("F");
    return checkResponseOk();
}

bool rn4020::getFirstFoundToken(tokenInfo* ti, int iTimeOut_ms)
{
    char* pCommaPrev=rx_buffer, *pCommaCur=NULL;
    char dummy[10];

    if(!getResponse(rx_buffer, iTimeOut_ms)){
        return false;
    }
    for(int i=0;i<4;i++){
        pCommaCur=strchr(pCommaPrev, ',');
        if(pCommaCur==NULL){
            return false;
        }
        switch(i){
        case 0:
            //get device address
            strncpy(ti->address, pCommaPrev, pCommaCur-pCommaPrev);
            break;
        case 1:
            //get Address type: public (=0) or random(=1)
            strncpy(dummy, pCommaPrev, pCommaCur-pCommaPrev);
            ti->addressType=atoi(dummy);
            break;
        case 2:
            //get friendlyname
            strncpy(ti->friendlyName, pCommaPrev, pCommaCur-pCommaPrev);
            break;
        case 3:
            //skip the UUIDs
            //get RSSI
            strcpy(dummy, pCommaCur+1);
            sscanf(dummy, "%x", &ti->rssi);
            break;
        }
        pCommaPrev=pCommaCur+1;
    }
    return true;
}

bool rn4020::stopScanningForDevices()
{
    sendCommand("X");
    return checkResponseOk();
}

bool rn4020::connect(tokenInfo *ti){
    char cmd[20];
    char addressType[2];
    strcpy(cmd,"E,");
    itoa(ti->addressType, addressType,10);
    strcat(cmd,addressType);
    strcat(cmd,",");
    strcat(cmd,ti->address);
    sendCommand(cmd);
    if(!checkResponseOk()){
        return false;
    }
    while(getResponse(rx_buffer, 9000))
    {
        if(strncmp(rx_buffer, CONNPARAM, strlen(CONNPARAM))==0){
            return true;
        }
        //        else{
        //            printf("ignored: %s\r\n", rx_buffer);
        //        }
    }
    return false;
}

bool rn4020::unboundPeripherals(){
    sendCommand("U");
    return checkResponseOk();
}


bool rn4020::checkResponseOk(){
    if(!getResponse(rx_buffer, 1000)){
        return false;
    }
    return strncmp(rx_buffer, AOK, strlen(AOK))==0;
}

void rn4020::sendCommand(const char* cmd)
{
    _uart1.rxClear();
    _uart1.write(cmd,strlen(cmd));
    _uart1.write('\r');
}


bool rn4020::getResponse(char* resp, int iTimeOut_ms)
{
    char* pResp = resp;
    Timer t;
    t.start();
    bool bEndOfLineFound=false;

    while(t.read_ms()<iTimeOut_ms){
        if(_uart1.readable())
        {
            _uart1.read(*pResp);
            if(*pResp == '\r')
            {
                *pResp='\0';
                bEndOfLineFound=true;
                break;
            }
            if(*pResp != '\n' && pResp-resp < RX_BUFFER_SIZE-1)
            {  //ignore newline characters
                pResp++;
            }
        }
    }
    t.stop();
    return bEndOfLineFound;
}
