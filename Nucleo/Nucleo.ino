//declaration of needed libraries must be done in the ino-file of the project.
#include "Arduino.h"
#include "nucleo.h"
#define HARDI2C
#include <Wire.h>
#include <SPI.h>
#ifndef ARDUINO_SAM_DUE
#include <EEPROM.h>
#endif

//git clone git@github.com:LieBtrau/PN532.git ~/git/PN532
//ln -s ~/git/PN532/PN532 ~/Arduino/libraries/
//ln -s ~/git/PN532/PN532_SPI/ ~/Arduino/libraries/
//Needed flash size: 1.3K
#include <PN532_SPI.h>
#include <PN532.h>

//git clone git@github.com:LieBtrau/NDEF.git ~/git/NDEF
//ln -s ~/git/NDEF/ ~/Arduino/libraries/
//Needed flash size: 0.8K
#include <NfcAdapter.h>

//git clone git@github.com:LieBtrau/RadioHead.git ~/git/RadioHead
//ln -s ~/git/RadioHead/ ~/Arduino/libraries/
//Needed flash size: 2.8K
#include "RadioHead.h"
#include <RH_MRF89XA.h>
#include <RHReliableDatagram.h>

//git clone git@github.com:LieBtrau/microBox.git ~/git/microBox
//ln -s ~/git/microBox/ ~/Arduino/libraries/
//Needed flash size: 10.4K
#include "microBox.h"

//~/git$ git clone git@github.com:LieBtrau/micro-ecc.git ~/git/uECC
//ln -s ~/git/uECC/ ~/Arduino/libraries/
//Needed flash size: 3.2K
#include "uECC.h"

//git clone git@github.com:adamvr/arduino-base64.git ~git/arduino-base64
//ln -s ~/git/arduino-base64 ~/Arduino/libraries/
#include "Base64.h"

//git clone git@github.com:LieBtrau/AES-CMAC-RFC.git ~/git/aes-cmac-rfc
//ln -s ~/git/aes-cmac-rfc/ ~/Arduino/libraries/
#include "cmac.h"

//git clone git@github.com:LieBtrau/arduino-ntag.git ~/git/arduino-ntag
//ln -s ~/git/arduino-ntag ~/Arduino/libraries/
#include "ntag.h"
#include "ntagsramadapter.h"

//git clone git@github.com:LieBtrau/arduino-nfc-sec-01.git ~/git/arduino-nfc-sec-01
//ln -s ~/git/arduino-nfc-sec-01 ~/Arduino/libraries/
#include "crypto.h"

//git clone git@github.com:thomasfredericks/Bounce2.git ~/git/bounce2
//ln -s ~/git/bounce2 ~/Arduino/libraries/
#include "Bounce2.h"

#include "nfcauthentication.h"

//git clone git@github.com:LieBtrau/Arduino_STM32.git ~/git/Arduino_STM32
//Remark that Arduino_STM32 doesn't seem to work with Arduino 1.6.7
//ln -s ~/git/Arduino_STM32/ ~/Programs/arduino-1.6.5/hardware/

//Build instruction:
//Adjust build.path to suit your needs.  Don't make it a subfolder of the directory where your *.ino 's are located,
//because Arduino 1.6.7 will compile these also, which will result in linking errors.
//~/Programs/arduino-1.6.5/arduino --verify --board Arduino_STM32:STM32F1:nucleo_f103rb --pref target_package=Arduino_STM32 --pref build.path=/home/ctack/build --pref target_platform=STM32F1 --pref board=nucleo_f103rb ~/Arduino/blinky_nucleo/blinky_nucleo.ino


char historyBuf[100];
char hostname[] = "ioBash";
char privateKey[32+1];//base64 string representation of 192bit key
char publicKey[64+1];//base64 string representation of 192bit key (x,y)
byte data[] = "HalloWereld!";
// Don't put this on the stack:
uint8_t buf[RH_MRF89XA_MAX_MESSAGE_LEN];

uint32_t ulStartTime;
uint32_t ulStartTime2;

#ifdef ARDUINO_SAM_DUE
PN532_SPI pn532spi(SPI, 10);
NfcAdapter nfc(pn532spi);
nfcAuthentication nfca(&nfc);
#else
Ntag ntag(Ntag::NTAG_I2C_1K,2,5);
NtagSramAdapter ntagAdapter(&ntag);
nfcAuthentication nfca(&ntagAdapter);
#endif
PARAM_ENTRY Params[]=
{
    {"hostname", hostname, PARTYPE_STRING | PARTYPE_RW, sizeof(hostname), NULL, NULL, 0},
    {"privateKey", privateKey, PARTYPE_STRING | PARTYPE_RW, sizeof(privateKey), NULL, NULL, 1},
    {"publicKey", publicKey, PARTYPE_STRING | PARTYPE_RW, sizeof(publicKey), NULL, NULL, 2},
    {NULL, NULL}
};

//  GND     = MRF89XAM8A: pin 1 => Arduino Uno pin GND
//  RST     = MRF89XAM8A: pin 2 => NC
//  /CSCON  = MRF89XAM8A: pin 3 => Arduino Uno pin 5
//  IRQ0    = MRF89XAM8A: pin 4 => Arduino Uno pin 4
//  SDI     = MRF89XAM8A: pin 5 => MOSI (Nucleo D11)
//  SCK     = MRF89XAM8A: pin 6 => SCK  (Nucleo D13)
//  SDO     = MRF89XAM8A: pin 7 => MISO (Nucleo D12)
//  /CSDATA = MRF89XAM8A: pin 8 => Arduino Uno pin 3
//  IRQ1    = MRF89XAM8A: pin 9 => Arduino Uno pin 2
//  VIN     = MRF89XAM8A: pin 10 => Arduino Uno pin 3V3
//RH_MRF89XA driver(3, 5, 4, 2);

//#define CLIENT_MRF89XA_RELIABLE
//#define SERVER_MRF89XA_RELIABLE
//#define CLIENT_MRF89XA_SIMPLE
//#define SERVER_MRF89XA_SIMPLE

#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 2

// Class to manage message delivery and receipt, using the driver declared above
#if defined(CLIENT_MRF89XA_RELIABLE)
#ifdef SERVER_MRF89XA_RELIABLE
#error You have to choose: client OR server, not both
#endif
RHReliableDatagram manager(driver, CLIENT_ADDRESS);
#elif defined(SERVER_MRF89XA_RELIABLE)
#ifdef CLIENT_MRF89XA_RELIABLE
#error You have to choose: client OR server, not both
#endif
RHReliableDatagram manager(driver, SERVER_ADDRESS);
#endif

//SCK => ICSP.3
//MISO => ICSP.1
//MOSI => ICSP.4
//SS => D6
//VCC => 3V3 (or 5V)
//GND => ICSP.6

void i2cRelease();

void setup() {
    ulStartTime2=ulStartTime=millis();
    Serial.begin(115200);
    Serial.println("start");
    i2cRelease();
    nfca.begin();
//    byte data[10];
//    NdefMessage message = NdefMessage();
//    message.addUnknownRecord(data,sizeof(data));
//    if(ntagAdapter.write(message)){
//        Serial.println("I²C has written message to tag.");
//    }
    //    if(base64_decode((char*)_localPrivateKey, pLocalPrivateKey, (uECC_BYTES<<2)/3) != uECC_BYTES)
    //    {
    //        return false;
    //    }
    //    if(base64_decode((char*)_localPublicKey, pLocalPublicKey, (uECC_BYTES<<3)/3) != uECC_BYTES*2)
    //    {
    //        return false;
    //    }

    //    bool bResult=cryptop.testMasterKeySse();
    //    Serial.print("Test master key Agreement + Derivation + Confirmation: ");
    //    Serial.println(bResult?"OK":"Fail");
    //    cryptop.cmacTest();
    //    if(!cryptop.setLocalKey(privateKey, publicKey)){
    //        Serial.println("Local keys not initialized.");
    //    }
#if defined(CLIENT_MRF89XA_RELIABLE) || defined(SERVER_MRF89XA_RELIABLE)
    if (!manager.init()){
        Serial.println("manager init failed");
        return;
    }
    Serial.println("manager init ok");
#elif defined(CLIENT_MRF89XA_SIMPLE) || defined(SERVER_MRF89XA_SIMPLE)
    if (!driver.init()){
        Serial.println("driver init failed");
        return;
    }
    Serial.println("driver init OK");
#endif
    microbox.begin(Params, hostname, true, historyBuf, sizeof(historyBuf));
}

void loop() {
    //    microbox.cmdParser();
    if(nfca.loop())
    {
        Serial.println("Pairing successful");
    }
//        if(millis()>ulStartTime2+3000){
//            ulStartTime2=millis();
//            NfcTag nf=ntagAdapter.read();
//            if(!nf.hasNdefMessage()){
//                return;
//            }
//            NdefMessage nfm=nf.getNdefMessage();
//            nfm.print();
//            NdefRecord ndf=nfm.getRecord(0);
//            byte dat[ndf.getPayloadLength()];
//            ndf.getPayload(dat);
//        }
#ifdef CLIENT_MRF89XA_RELIABLE
    Serial.println("Sending to mrf89xa_reliable_datagram_server");

    // Send a message to manager_server
    if (manager.sendtoWait(data, sizeof(data), SERVER_ADDRESS))
    {
        // Now wait for a reply from the server
        uint8_t len = sizeof(buf);
        uint8_t from;
        if (manager.recvfromAckTimeout(buf, &len, 2000, &from))
        {
            Serial.print("got reply from : 0x");
            Serial.print(from, HEX);
            Serial.print(": ");
            Serial.println((char*)buf);
        }
        else
        {
            Serial.println("No reply, is mrf89xa_reliable_datagram_server running?");
        }
    }

#elif defined(SERVER_MRF89XA_RELIABLE)
    if (manager.available())
    {
        // Wait for a message addressed to us from the client
        uint8_t len = sizeof(buf);
        uint8_t from;
        if (manager.recvfromAck(buf, &len, &from))
        {
            Serial.print("got request from : 0x");
            Serial.print(from, HEX);
            Serial.print(": ");
            Serial.println((char*)buf);

            // Send a reply back to the originator client
            if (!manager.sendtoWait(data, sizeof(data), from))
                Serial.println("sendtoWait failed");
        }else{
            Serial.println("not valid");
        }
    }
#elif defined(CLIENT_MRF89XA_SIMPLE)
    Serial.println("Sending to mrf89xa");
    // Send a message to mrf89xa
    driver.send(data, sizeof(data));

    driver.waitPacketSent();
    // Now wait for a reply
    uint8_t len = sizeof(buf);

    if (driver.waitAvailableTimeout(500))
    {
        // Should be a reply message for us now
        if (driver.recv(buf, &len))
        {
            Serial.print("got reply: ");
            Serial.println((char*)buf);
        }
        else
        {
            Serial.println("recv failed");
        }
    }
    else
    {
        Serial.println("No reply, is mrf89xa_server running?");
    }
    delay(400);
#elif defined(SERVER_MRF89XA_SIMPLE)
    if (driver.available())
    {
        // Should be a message for us now
        uint8_t len = sizeof(buf);
        if (driver.recv(buf, &len))
        {
            //      NRF24::printBuffer("request: ", buf, len);
            Serial.print("got request: ");
            Serial.println((char*)buf);

            // Send a reply
            uint8_t data[] = "And hello back to you";
            driver.send(data, sizeof(data));
            driver.waitPacketSent();
            Serial.println("Sent a reply");
        }
        else
        {
            Serial.println("recv failed");
        }
    }
#endif
}


void i2cRelease()
{
#ifdef ARDUINO_STM_NUCLEU_F103RB
    const byte SCL_PIN=PB8;
#elif defined ARDUINO_SAM_DUE
    const byte SCL_PIN=21;
#endif
    pinMode(SCL_PIN, OUTPUT);
    for(byte i=0;i<100;i++)
    {
        digitalWrite(SCL_PIN, LOW);
        delayMicroseconds(10);
        digitalWrite(SCL_PIN, HIGH);
        delayMicroseconds(10);
    }
    delayMicroseconds(1000);
}

