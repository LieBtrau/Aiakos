/* STM32L053 Nucleo connections to RN4020 module:
 *  Nucleo.PC_11 = RN4020.5 = yellow (TX of RN4020)
 *  Nucleo.PC_10 = RN4020.6 = orange (RX of RN4020)
 *  Nucleo.A1 RTS (out) = RN4020.14 CTS (in) = green
 *  Nucleo.A0 CTS (in)  = RN4020.18 RTS (out) = brown
 */

#include "mbed.h"
#include "rn4020.h"
#include <string>
#include "../../Atmel/RadioHead/RadioHead.h"
#include "../../Atmel/RadioHead/RH_Serial.h"
#include "hardwareserial.h"

Serial pc(USBTX, USBRX);
HardwareSerial hs(pc);
RH_Serial driver(hs);
rn4020 rn(PC_10, PC_11, PA_1, PA_0);

bool bindNewPebbleBee(rn4020::tokenInfo *ti);

int main()
{
    //rn4020::tokenInfo ti;
    char c;
    bool bStarted=false;
    string strIn;

    pc.baud(115200);
    //bindNewPebbleBee(&ti);
    pc.printf("\r\nready...\r\n");
    while(1){
        if(pc.readable()) {
            c=pc.getc();
            if(!bStarted) {
                strIn+=c;
                if(strIn.find("start")!= string::npos) {
                    bStarted=true;
                    pc.printf("started now...\r\n");
                }
            } else {
                rn.write(c);
            }
        }
        if(rn.read(c)) {
            pc.putc(c);
        }
    }
}


bool bindNewPebbleBee(rn4020::tokenInfo* ti){
    //rn.setEchoOn(false);
    if(rn.rebootModule()){
        printf("module rebooted\r\n");
    }
    if(rn.unboundPeripherals()){
        printf("Peripherals unbound\r\n");
    }
    if(rn.startScanningForDevices()){
        printf("scanning started\r\n");
    }
    if(rn.getFirstFoundToken(ti, 8000)){
        printf("token found: %s, RSSI: %d\r\n", ti->address, ti->rssi);
    }
    if(rn.stopScanningForDevices()){
        printf("scanning stopped\r\n");
    }
    if(!rn.isPebbleBee(ti)){
        return false;
    }
    printf("oh yeah, I caught a PebbleBee!\r\n");
    if(rn.connect(ti)){
        printf("Connected!\r\n");
    }
    if(!rn.setCharacteristic(0x0B, 0xAA)){
        return false;
    }
    printf("0xB set\r\n");
    if(!rn.setCharacteristic(0x0E, 0xAA)){
        return false;
    }
    printf("0xE set\r\n");
    return true;
}
