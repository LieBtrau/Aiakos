/*
 */

#include <RH_Serial.h>          //for wired comm
#include "blepairing.h"

namespace
{
RH_Serial rhSerial(Serial);     //UART1
RHReliableDatagram mgrSerial(rhSerial, 3);
BlePairing ble(&mgrSerial, 1);
HardwareSerial* debugSerial;
}

void setup() {
    debugSerial=&Serial1;       //UART2
    pinMode(13, OUTPUT);
    debugSerial->begin(9600);
    rhSerial.serial().begin(1200);
    if (!mgrSerial.init())
    {
        debugSerial->println("init failed");
    }
}

// the loop function runs over and over again forever
void loop() {
    digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(500);              // wait for a second
    digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
    delay(500);              // wait for a second
}
