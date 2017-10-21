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
bool readDataSer(byte* data, byte& length);
bool writeDataSer(byte* data, byte length);

namespace
{
const byte RFID_KEY_SIZE=5;
const byte CABLE_DETECT_PIN=6;
const byte BUTTON_PIN=25;
byte rfid_key[RFID_KEY_SIZE]={0xAA,0xAB,0xAC,0xAD,0xAE};
RH_Serial rhSerial(Serial2);                    //UART3
RHReliableDatagram mgrSerial(rhSerial, 3);
RHReliableDatagram* pmgrSer;
/* WAKE_SW (7)  ->  3
 * BT_ACTIVE(12)->  4
 * WAKE_HW(15)  ->  5
 * enPwr        ->  7
 */
rn4020 rn(Serial1, 3, 4, 5, 7);                 //UART1, for short range wireless
bleControl ble(&rn);
blePairingCentral blePair(writeDataSer, readDataSer, &ble, RFID_KEY_SIZE);
byte peerAddress;
bool bConnected;
unsigned long buttonTimer=0;
Bounce cableDetect=Bounce();
Bounce pushButton=Bounce();
btCharacteristic ble_char_rfid("f1a87912-5950-479c-a5e5-b6cc81cd0502",        //private service
                          "855b1938-83e2-4889-80b7-ae58fcd0e6ca",        //private characteristic
                          btCharacteristic::WRITE_WOUT_RESP,5,           //properties+length
                          btCharacteristic::ENCR_W);                     //security
btCharacteristic ble_char_ias_alertLevel("1802",                                  //IAS Alert Service
                                "2A06",                                  //Alert Level characteristic
                                btCharacteristic::WRITE_WOUT_RESP, 1,    //properties+length
                                btCharacteristic::NOTHING                //security
                                );
}

void setup()
{
    openDebug(9600);
    rhSerial.serial().begin(2400);
    pmgrSer=&mgrSerial;
    peerAddress=2;                  //keyfob has address 2
    if(!pmgrSer->init())
    {
        debug_println("init failed");
    }
    pinMode(CABLE_DETECT_PIN, INPUT_PULLUP);
    cableDetect.attach(CABLE_DETECT_PIN);
    cableDetect.interval(100); // interval in ms
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pushButton.attach(BUTTON_PIN);
    pushButton.interval(100); // interval in ms
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
    pushButton.update();
    if(!cableDetect.read())
    {
        //Secure pairing mode
        switch(blePair.loop())
        {
        case BlePairing::AUTHENTICATION_OK:
            debug_println("Securely paired");
            break;
        case BlePairing::NO_AUTHENTICATION:
            pmgrSer->resetDatagram();
            break;
        case BlePairing::AUTHENTICATION_BUSY:
            break;
        }
    }else
    {
        //Keyfob wake up mode
        ble.loop();
        if(pushButton.fell())
        {
            buttonTimer=millis();
        }
        if(pushButton.rose())
        {
            if(millis()-buttonTimer<1000)
            {
                //open door

                if(ble.secureConnect(blePair.getRemoteBleAddress()))
                {
                    if(ble.writeRemoteCharacteristic(&ble_char_rfid, rfid_key,RFID_KEY_SIZE))
                    {
                        debug_println("RFID Value written");
                    }
                    ble.disconnect();
                }
            }
            else
            {
                //find key
                if(ble.secureConnect(blePair.getRemoteBleAddress()))
                {
                    byte array[1]={0xBB};
                    if(ble.writeRemoteCharacteristic(&ble_char_ias_alertLevel, array,1))
                    {
                        debug_println("Alert value written");
                    }
                }
            }
        }
    }
}


bool writeDataSer(byte* data, byte length)
{
    debug_print("Sending serial data...");
    debug_printArray(data, length);
    return pmgrSer->sendtoWait(data, length, peerAddress);
}

bool readDataSer(byte *data, byte& length)
{
    byte from;
    if (!pmgrSer->available())
    {
        return false;
    }
    if(!pmgrSer->recvfromAck(data, &length, &from))
    {
        return false;
    }
    if(from != peerAddress)
    {
        debug_print("Sender doesn't match");
        return false;
    }
    debug_print("Received data: ");debug_printArray(data, length);
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
    if(!ble.init(115200))
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
    rn.getRemoteHandle(&ble_char_rfid);
    blePair.init(rfid_key);
    return ble.beginCentral();
}

