/* Source code for RN4020 can be found on: http://ww1.microchip.com/downloads/en/DeviceDoc/rn4020_pictail__pic18_usb_uart_firmware.zip
 */

#include "rn4020.h"
#include <cstring>

char rx_buffer[64];
static const char* REBOOT="Reboot";
static const char* CMD="CMD";
static const char* ECHO_ON="Echo On";
static const char* ECHO_OFF="Echo Off";
static const char* AOK="AOK";

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

bool rn4020::getFirstFoundToken(char* foundToken, int iTimeOut_ms)
{
    char* pComma;
    if(!getResponse(rx_buffer, iTimeOut_ms)){
        return false;
    }
    pComma=strchr(rx_buffer, ',');
    if(pComma==0){
        return false;
    }
    strncpy(foundToken, rx_buffer, pComma-rx_buffer);
    return true;
}


bool rn4020::stopScanningForDevices()
{
    sendCommand("X");
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
    _uart1.rxClear();  //clear RX-buffer before sending a new command.
    _uart1.write(cmd,strlen(cmd));
    _uart1.write('\r');
}


bool rn4020::getResponse(char* resp, int iTimeOut_ms)
{
    char* pResp = resp;
    Timer t;
    t.start();

    while(t.read_ms()<iTimeOut_ms){
        if(_uart1.readable())
        {
            _uart1.read(*pResp);
            if(*pResp == '\r')
                break;
            if(*pResp != '\n')  //ignore newline characters
                pResp++;
        }
    }
    t.stop();
    return pResp-resp!=0;
}
