#include "Arduino.h"
#include "RadioHead.h"
#include <RH_MRF89XA.h>
#include <RHReliableDatagram.h>
#include <PN532_SPI.h>
#include <NfcAdapter.h>
#include "microBox.h"
#include "crypto.h"
#include "ntag.h"
#include "ntagsramadapter.h"

char historyBuf[100];
char hostname[] = "ioBash";
char privateKey[32+1];//base64 string representation of 192bit key
char publicKey[64+1];//base64 string representation of 192bit key (x,y)

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

byte data[] = "HalloWereld!";
// Don't put this on the stack:
uint8_t buf[RH_MRF89XA_MAX_MESSAGE_LEN];
PN532_SPI pn532spi(SPI, 10);
NfcAdapter nfc = NfcAdapter(pn532spi);
Crypto cryptop;
Ntag ntag(Ntag::NTAG_I2C_1K);
NtagSramAdapter ntagAdapter(&ntag);
uint32_t ulStartTime;
uint32_t ulStartTime2;

void i2cRelease();
void readerLoop();
void tagLoop(bool bInitialize=false);

void setup() {
    ulStartTime2=ulStartTime=millis();
    Serial.begin(115200);
    Serial.println("start");
    i2cRelease();
    nfc.begin();
    ntagAdapter.begin();
    tagLoop(true);
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
    readerLoop();
    tagLoop();
    //    if(millis()>ulStartTime2+3000){
    //        ulStartTime2=millis();
    //        cryptop.eccTest();
    //    }
    /*
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
    */
}

void tagLoop(bool bInitialize){
    byte id=0;
    if(!bInitialize){
        if(!ntag.waitUntilNdefRead(10)){
            return;
        }
        NfcTag nf=ntagAdapter.read();
        if(!nf.hasNdefMessage()){
            return;
        }
        NdefMessage nfm=nf.getNdefMessage();
        nfm.print();
        NdefRecord ndf=nfm.getRecord(0);
        byte dat[ndf.getPayloadLength()];
        ndf.getPayload(dat);
        for(byte i=0;i<ndf.getPayloadLength();i++){
            Serial.print(dat[i], HEX);Serial.print(" ");
        }
        Serial.println();
        id=dat[0];
    }
    if(bitRead(id,0)==0){
        data[0]=id+1;
        NdefMessage message = NdefMessage();
        message.addUnknownRecord(data,sizeof(data));
        if(ntagAdapter.write(message)){
            Serial.println("Tag has written message written to tag.");
        }
    }
}

void readerLoop()
{
    if(!nfc.tagPresent()){
        return;
    }
    NfcTag tag = nfc.read();
    if(!tag.hasNdefMessage()){
        return;
    }
    NdefMessage nfm=tag.getNdefMessage();
    nfm.print();
    NdefRecord ndf=nfm.getRecord(0);
    byte dat[ndf.getPayloadLength()];
    ndf.getPayload(dat);
    for(byte i=0;i<ndf.getPayloadLength();i++){
        Serial.print(dat[i], HEX);Serial.print(" ");
    }
    Serial.println();
    if(bitRead(dat[0],0)){
        NdefMessage message = NdefMessage();
        data[0]=dat[0]+1;
        message.addUnknownRecord(data,sizeof(data));
        if(nfc.write(message)){
            Serial.println("Reader has written message to tag.");
        }
    }
}

void i2cRelease()
{
    pinMode(PB8, OUTPUT);
    for(byte i=0;i<20;i++)
    {
        digitalWrite(PB8, LOW);
        delay_us(10);
        digitalWrite(PB8, HIGH);
        delay_us(10);
    }
}

