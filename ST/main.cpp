/* STM32L053 Nucleo connections to RN4020 module:
 *  Nucleo.PC_11 = RN4020.5 = yellow (TX of RN4020)
 *  Nucleo.PC_10 = RN4020.6 = orange (RX of RN4020)
 *  Nucleo.A1 = RN4020.14 = green
 *  Nucleo.A0 = RN4020.18 = brown
 */
#include "mbed.h"
#include "rn4020.h"
Serial pc(USBTX, USBRX);
rn4020 rn(PC_10, PC_11, PA_1, PA_0);

bool bindNewPebbleBee(rn4020::tokenInfo *ti);

int main()
{
    rn4020::tokenInfo ti;
    pc.baud(115200);
    bindNewPebbleBee(&ti);
    while(1){
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
