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

int main()
{
    rn4020::tokenInfo ti;
    pc.baud(115200);
    bool bFound=false;
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
    if(rn.getFirstFoundToken(&ti, 5000)){
        printf("token found: %s, RSSI: %d\r\n", ti.address, ti.rssi);
    }
    if(rn.isPebbleBee(&ti)){
        printf("oh yeah, I caught a PebbleBee!\r\n");
        bFound=true;
    }
    if(rn.stopScanningForDevices()){
        printf("scanning stopped\r\n");
    }
    if(bFound){
        if(rn.connect(&ti)){
            printf("Connected!\r\n");
        }
    }
    while(1){
    }
}
