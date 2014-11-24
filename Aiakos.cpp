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

rdm630 rfid(6, 0);  //TX-pin of RDM630 connected to Arduino pin 6
SHA204I2C sha204dev(0xC0);//ATECC108 instead of ATSHA204


uint8_t private_mr_key[NUM_ECC_DIGITS] = {
    0x9B, 0x26, 0x2A, 0xF2, 0x70, 0x34, 0x46, 0xFF,
    0xB2, 0xCA, 0x7A, 0x3C, 0xC1, 0x39, 0x87, 0x5E,
    0x0E, 0xE0, 0xF7, 0x0F, 0x02, 0x3B, 0x05, 0xE9};
EccPoint public_mr_key = {
    {0x15, 0x73, 0xDD, 0x10, 0x9E, 0x4D, 0x57, 0xD1,
     0xFB, 0x17, 0x06, 0xFC, 0xC8, 0xF7, 0xCD, 0xD2,
     0x03, 0xAF, 0x0B, 0x7F, 0x40, 0xB6, 0xA9, 0x93},
    {0xB9, 0xE3, 0xE4, 0x24, 0x4F, 0x38, 0x48, 0xE9,
     0x13, 0x5E, 0xDD, 0xA9, 0x6C, 0xFB, 0xD2, 0xC7,
     0x20, 0xD4, 0x01, 0x4A, 0x91, 0x41, 0xDB, 0xB9}};
uint8_t randomnr[NUM_ECC_DIGITS] = {
    0x78, 0xE7, 0x17, 0x78, 0x4D, 0xF5, 0x0A, 0xB1,
    0x66, 0x29, 0xE0, 0xE4, 0xDC, 0x4E, 0x6B, 0x17,
    0xCA, 0xC6, 0x45, 0xF0, 0x01, 0x26, 0x86, 0x25};
uint8_t hash[NUM_ECC_DIGITS] = {
     0x3F, 0x65, 0xCB, 0x21, 0x19, 0x66, 0x31, 0x48,
     0x5B, 0x8B, 0x65, 0x5F, 0xA8, 0x0C, 0x4A, 0xD5,
     0xC6, 0xA6, 0x43, 0x69, 0x61, 0x7B, 0xD0, 0xC9};


void setup()
{
    Serial.begin(9600);  // start serial to PC
    rfid.begin();
    sha204dev.init();
//    uint8_t r[NUM_ECC_DIGITS], s[NUM_ECC_DIGITS];
//    unsigned long ulstartTime=millis();
//    if(!ecdsa_sign(r,s,private_mr_key,randomnr,hash)){
//        return;
//    }
//    Serial.println("Signature generated OK");
//    Serial.println(millis()-ulstartTime);
//    ulstartTime=millis();
//    if(!ecdsa_verify(&public_mr_key,hash,r,s)){
//        return;
//    }
//    Serial.println("Signature is OK");
//    Serial.println(millis()-ulstartTime);

  Serial.println(millis());
//  for (int i=0; i<12; i++) {
//    Serial.println("Sleep!");
//      sha204dev.sleep();
//    delay(3000);
//    Serial.println("Wake!");

    Serial.println(sha204dev.wakeup(), HEX);
    Serial.println("----------");
    //Serial.println("Testing Chip");
    //Serial.println();
    //Serial.println("Sending a Wakup Command. Response should be:\r\n4 11 33 43:");
    //Serial.println("Response is:");
    //wakeupExample();
    //delay(500);
    Serial.println();
    Serial.println("Asking the SHA204's serial number. Response should be:");
    Serial.println("1 23 x x x x x x x EE");
    Serial.println("Response is:");
    serialNumberExample();
    //Serial.println();
    //Serial.println("Sending a MAC Challenge. Response should be:");
    //Serial.println("23 6 67 0 4F 28 4D 6E 98 62 4 F4 60 A3 E8 75 8A 59 85 A6 79 96 C4 8A 88 46 43 4E B3 DB 58 A4 FB E5 73");
    //Serial.println("Response is:");
    //macChallengeExample();
//  }
  Serial.println(millis());
  Serial.println("Done!");

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

byte serialNumberExample() {
  uint8_t serialNumber[9];
  byte returnValue;

  returnValue = sha204dev.serialNumber(serialNumber);
  for (int i=0; i<9; i++) {
    Serial.print(serialNumber[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  Serial.println("-------");

  return returnValue;
}

byte macChallengeExample() {
  uint8_t command[MAC_COUNT_LONG];
  uint8_t response[MAC_RSP_SIZE];

  const uint8_t challenge[MAC_CHALLENGE_SIZE] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
  };

  uint8_t ret_code = sha204dev.execute(SHA204_MAC, 0, 0, MAC_CHALLENGE_SIZE,
    (uint8_t *) challenge, 0, NULL, 0, NULL, sizeof(command), &command[0],
    sizeof(response), &response[0]);

  for (int i=0; i<SHA204_RSP_SIZE_MAX; i++) {
    Serial.print(response[i], HEX);
    Serial.print(' ');
  }
  Serial.println();

  return ret_code;
}
