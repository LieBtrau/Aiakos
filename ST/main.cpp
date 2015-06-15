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
char foundToken[13];

int main()
{
    pc.baud(115200);
    if(rn.rebootModule()){
        printf("module rebooted\r\n");
    }
    if(rn.startScanningForDevices()){
        printf("scanning started\r\n");
    }
    if(rn.getFirstFoundToken(foundToken, 3000)){
        printf("token found: %s\r\n", foundToken);
    }
    if(rn.stopScanningForDevices()){
        printf("scanning stopped\r\n");
    }
    while(1){
    }
    /*    while(1) {
        if(pc.readable()) {
            c=pc.getc();
            //            if(!bStarted) {
            //                strIn+=c;
            //                if(strIn.find("start")!= string::npos) {
            //                    bStarted=true;
            //                    pc.printf("started now...\n");
            //                }
            //            } else {
            uart1.write(c);
        }
        //    }
        if(uart1.read(c)) {
            pc.putc(c);
        }
    }
    */
}
