/*Connections between Nucleo and RN4020, on Nucleo Serial2 is connected to the debugger
 * RN4020.1 (GND)       -> GND
 * RN4020.5 (TX)        -> D2 (Serial1_RX)
 * RN4020.6 (RX)        -> D8 (Serial1_TX)
 * RN4020.7 (WAKE_SW)   -> D3
 * RN4020.12 (ACT)      -> D4
 * RN4020.15 (WAKE_HW)  -> D5
 * RN4020.PWREN         -> D6
 * RN4020.23 (3V3)      -> 3V3
 */

#include <RH_Serial.h>          //for wired comm
#include "blepairingcentral.h"
#include "debug.h"
#include "blecontrol.h"
#include <Bounce2.h>            //for switch debouncing

void bleEvent(bleControl::EVENT ev);
bool readDataSer(byte** data, byte& length);
bool writeDataSer(byte* data, byte length);

namespace
{
RH_Serial rhSerial(Serial2);                    //UART3
RHReliableDatagram mgrSerial(rhSerial, 3);
RHReliableDatagram* pmgrSer;
rn4020 rn(Serial1, 3, 4, 5, 7);                 //UART1, for short range wireless
bleControl ble(&rn);
blePairingCentral blePair(writeDataSer, readDataSer, &ble);
byte peerAddress;
bool bConnected;
Bounce cableDetect;
const byte CABLE_DETECT_PIN=6;
}

void setup() {
    openDebug(9600);
    rhSerial.serial().begin(2400);
    pmgrSer=&mgrSerial;
    peerAddress=2;                  //keyfob has address 2
    if (!pmgrSer->init())
    {
        debug_println("init failed");
    }
    pinMode(CABLE_DETECT_PIN, INPUT_PULLUP);
    cableDetect.attach(CABLE_DETECT_PIN);
    cableDetect.interval(100); // interval in ms
    ble.setEventListener(bleEvent);
    if(!initBleCentral())
    {
        debug_println("Ble init failed.");
        return;
    }
    debug_print("All ok");
}

// the loop function runs over and over again forever
void loop()
{
    cableDetect.update();
    if(!cableDetect.read())
    {
        //Secure pairing mode
        switch(blePair.loop())
        {
        case BlePairing::AUTHENTICATION_OK:
            debug_println("Securely paired");
            break;
        case BlePairing::NO_AUTHENTICATION:
        case BlePairing::AUTHENTICATION_BUSY:
            break;
        }
    }else
    {
        //Keyfob wake up mode
    }
}

bool writeDataSer(byte* data, byte length)
{
    debug_print("Sending serial data...");
    debug_printArray(data, length);
    return pmgrSer->sendtoWait(data, length, peerAddress);
}

bool readDataSer(byte** data, byte& length)
{
    byte from;
    if (!pmgrSer->available())
    {
        return false;
    }
    if(!pmgrSer->recvfromAck(*data, &length, &from))
    {
        return false;
    }
    if(from != peerAddress)
    {
        debug_print("Sender doesn't match");
        return false;
    }
    debug_print("Received data: ");debug_printArray(*data, length);
    return true;
}

void bleEvent(bleControl::EVENT ev)
{
    switch(ev)
    {
    case bleControl::EV_PASSCODE_GENERATED:
        blePair.eventPasscodeGenerated();
        break;
    case bleControl::EV_CONNECTION_DOWN:
        debug_println("Connection down");
        bConnected=false;
        break;
    case bleControl::EV_CONNECTION_UP:
        debug_println("Connection up");
        bConnected=true;
        break;
    default:
        debug_print("Unknown event: ");
        debug_println(ev, DEC);
        break;
    }
}

bool initBleCentral()
{
    char dataname[20];
    const char BT_NAME_BIKE[]="AiakosBike";
    if(!ble.init())
    {
        debug_println("RN4020 not set up");
        return false;
    }
    if(!ble.getBluetoothDeviceName(dataname))
    {
        return false;
    }
    if(strncmp(dataname,BT_NAME_BIKE, strlen(BT_NAME_BIKE)))
    {
        //Module not yet correctly configured
        if(!ble.programCentral())
        {
            return false;
        }
        if(!ble.setBluetoothDeviceName(BT_NAME_BIKE))
        {
            return false;
        }
    }
    return ble.beginCentral();
}



