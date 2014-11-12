/*  Garage door opener + remote control application
    Copyright (C) 2014  Christoph Tack

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

//Howto: Setting up the build environment: Arduino & QtCreator: http://lucidarme.me/?p=3282
//  http://blag.pseudoberries.com/post/16374999524/avr-debugwire-on-linux
//  http://awtfy.com/2012/03/29/hardware-debugging-the-arduino-using-eclipse-and-the-avr-dragon/

#include "Aiakos.h"
#include "ATECC108/atecc108.h"

rdm630 rfid(6, 0);  //TX-pin of RDM630 connected to Arduino pin 6
atecc108 crypto;

// data for challenge in MAC mode 0 command
const uint8_t challenge[32] = {
    0x00, 0x12, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
};


void setup()
{
    Serial.begin(9600);  // start serial to PC
    rfid.begin();
    ecc108p_init();
    Serial.print("Result of MAC-calculation: ");

//    byte response_mac[crypto.MAC_RSPSIZE];
//    if(crypto.generateMac(challenge, crypto.ECC108_KEY_ID, response_mac)!=0){
//        Serial.println("Can't generate MAC");
//        return;
//    }

//    if(crypto.checkmac(challenge, response_mac,crypto.ECC108_KEY_ID)!=0){
//        Serial.println("Verify MAC failed.");
//        return;
//    }
//    Serial.println("MAC generated & verified successfully.");

    byte sn[9];
    if(crypto.getSerialNumber(sn))
    {
        for(int i=0;i<9;Serial.println(sn[i++], HEX));
    }
}

void loop()
{
    byte data[6];
    byte length;

    if(rfid.available()){
        rfid.getData(data,length);
        Serial.println("Data valid");
        for(int i=0;i<length;i++){
            Serial.print(data[i],HEX);
        }
        Serial.println();
    }
}
