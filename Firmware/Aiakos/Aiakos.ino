/* Hardware Connections
 * **************************************************
 * LoRa module
 * **************************************************
 * XL1276    Protrinket  Due    Nucleo   ATSHA204A
 *           3V
 *****************************************************
 * NSS       10          4       A2
 * MOSI      11          ICSP.4  D11
 * MISO      12          ICSP.1  D12
 * SCK       13          ICSP.3  D13  (Be careful, this is the 6th pin on the left row of the right most pin header connector, not the fifth!)
 * REST      RST         RESET   NRST
 * DIO0      3           3       D5
 * VCC       3V          3.3V    3V
 * GND       G           GND     GND
 *
 *****************************************************
 * ATSHA204A for TRNG and unique serial number
 *****************************************************
 *                               D3      SDA
 *                               D4      SCL
 *
 *****************************************************
 * Serial connection for secure pairing
 *****************************************************
 * Tip                   TX1     D2
 * Ring                  RX1     D8
 * Sleeve                GND     GND
 * Cable detect Contact  2       D6
 * Pushbutton                    25
 *
 *****************************************************
 * Pulse output for opening/closing garage door
 *****************************************************
 * Pulse                 5
 *
 * Pairing procedure:
 * ------------------
 * Interconnect the keyfob and the garage controller with the stereo jack cable.  Press the blue button on the Nucleo
 * to start the pairing.
 *
 * Authenticating procedure:
 * -------------------------
 * Disconnect the stereo jack cable from the key fob as well as from the garage controller.  Press the blue button on
 * the Nucleo to initiate authentication.
*/

#include <RHReliableDatagram.h> //for wireless comm
#include <RH_RF95.h>            //for wireless comm
#include <RH_Serial.h>          //for wired comm
#include <SPI.h>                //for wireless comm
#include "kryptoknightcomm.h"   //for authentication
#include "ecdhcomm.h"           //for secure pairing
#include "configuration.h"      //for non-volatile storage of parameters
#include "cryptohelper.h"       //for unique serial numbers & true random number generators
#include <Bounce2.h>            //for switch debouncing

#define DEBUG

const byte ADDRESS1=1;
const byte ADDRESS2=2;

bool writeDataLoRa(byte* data, byte length);
bool readDataLoRa(byte** data, byte& length);
bool readDataSer(byte** data, byte& length);
bool writeDataSer(byte* data, byte length);

RH_Serial rhSerial(Serial1);
Configuration cfg;
KryptoKnightComm k= KryptoKnightComm(&RNG, writeDataLoRa, readDataLoRa);
EcdhComm ecdh= EcdhComm(&RNG, writeDataSer, readDataSer);
Bounce cableDetect = Bounce();

#ifdef ARDUINO_STM_NUCLEO_F103RB
#define ROLE_KEYFOB
#elif defined(ARDUINO_SAM_DUE)
#define ROLE_GARAGE_CONTROLLER
#else
#error No device type defined.
#endif

#ifdef ARDUINO_STM_NUCLEO_F103RB
RH_RF95 rhLoRa(A2,5);//NSS, DIO0
const byte CABLE_DETECT_PIN=6;
Bounce pushButton = Bounce();
#elif defined(ARDUINO_SAM_DUE)
RH_RF95 rhLoRa(4,3);
const byte CABLE_DETECT_PIN=2;
#endif

#ifdef ROLE_GARAGE_CONTROLLER
RHReliableDatagram mgrLoRa(rhLoRa, ADDRESS1);
RHReliableDatagram mgrSer(rhSerial, ADDRESS1);
const byte PULSE_PIN=5;
#elif defined(ROLE_KEYFOB)
byte payload[4]={0xFE, 0xDC, 0xBA, 0x98};
RHReliableDatagram mgrLoRa(rhLoRa, ADDRESS2);
RHReliableDatagram mgrSer(rhSerial, ADDRESS2);
const byte BUTTON_PIN=25;
#endif
static unsigned long ulTime;

void setup()
{
    Serial.begin(9600);
    rhSerial.serial().begin(2400);
    while (!Serial) ; // Wait for serial port to be available
    if ((!mgrLoRa.init()))
    {
#ifdef DEBUG
        Serial.println("LoRa init failed");
#endif
        return;
    }
    if ((!mgrSer.init()))
    {
#ifdef DEBUG
        Serial.println("Serial init failed");
#endif
        return;
    }
#ifdef ARDUINO_SAM_DUE
    initRng();
#endif
    pinMode(CABLE_DETECT_PIN, INPUT_PULLUP);
    cableDetect.attach(CABLE_DETECT_PIN);
    cableDetect.interval(5); // interval in ms
    byte buf[10];
    if((!getSerialNumber(buf, Configuration::IDLENGTH))
            || (!k.init(buf,Configuration::IDLENGTH) )
            || (!ecdh.init(buf, Configuration::IDLENGTH)) )
    {
        return;
    }
    if(cfg.init())
    {
#ifdef DEBUG
        Serial.println("Config valid");
#endif
#ifdef ROLE_GARAGE_CONTROLLER
        k.setMessageReceivedHandler(dataReceived);
        k.setKeyRequestHandler(setKeyInfo);
        pinMode(PULSE_PIN, OUTPUT);
#elif defined(ROLE_KEYFOB)
        pinMode(BUTTON_PIN, INPUT_PULLUP);
        pushButton.attach(BUTTON_PIN);
        pushButton.interval(5); // interval in ms
#endif
    }

#ifdef DEBUG
    Serial.println("ready");
#endif
    ulTime=millis();
}

#ifdef ROLE_KEYFOB
void loop()
{
    cableDetect.update();
    pushButton.update();
    if(!cableDetect.read())
    {
        //Secure pairing mode
        if(pushButton.fell())
        {
            k.reset();
            if(!ecdh.startPairing())
            {
                Serial.println("Sending message failed.");
                return;
            }
        }
        switch(ecdh.loop())
        {
        case EcdhComm::AUTHENTICATION_OK:
#ifdef DEBUG
            Serial.println("Securely paired");
#endif
            cfg.addKey(ecdh.getRemoteId(), ecdh.getMasterKey());
            break;
        case EcdhComm::NO_AUTHENTICATION:
        case EcdhComm::AUTHENTICATION_BUSY:
            break;
        }
    }else
    {
        //Authenticating mode
        if(pushButton.fell())
        {
            ecdh.reset();
            Serial.println("Initiator starts authentication");
            if(!k.sendMessage(payload,sizeof(payload), cfg.getDefaultId(), cfg.getIdLength(), cfg.getDefaultKey()))
            {
                Serial.println("Sending message failed.");
                return;
            }
        }
        if(k.loop()==KryptoKnightComm::AUTHENTICATION_AS_INITIATOR_OK)
        {
            Serial.println("Message received by peer and acknowledged");
        }
    }
}

#elif defined(ROLE_GARAGE_CONTROLLER)
void loop()
{
    cableDetect.update();
    if(!cableDetect.read())
    {
        //Secure pairing mode
        if (ecdh.loop() == EcdhComm::AUTHENTICATION_OK)
        {
#ifdef DEBUG
            Serial.println("Securely paired");
#endif
            cfg.addKey(ecdh.getRemoteId(), ecdh.getMasterKey());
        }
    }else
    {
        if(k.loop()==KryptoKnightComm::AUTHENTICATION_AS_PEER_OK)
        {
            Serial.println("Message received by remote initiator");
        }
    }
    if(millis()>ulTime+2000)
    {
        ulTime=millis();
        digitalWrite(PULSE_PIN, HIGH);
        delayMicroseconds(10);
        digitalWrite(PULSE_PIN, LOW);
        Serial.println("0");
    }
}
#endif


bool writeDataSer(byte* data, byte length)
{
    Serial.print("Sending serial data...");
#ifdef ROLE_GARAGE_CONTROLLER
#ifdef DEBUG
    Serial.println("to key fob: ");print(data, length);
#endif
    return mgrSer.sendtoWait(data, length, ADDRESS2);
#elif defined(ROLE_KEYFOB)
#ifdef DEBUG
    Serial.println("to garage controller: ");print(data, length);
#endif
    return mgrSer.sendtoWait(data, length, ADDRESS1);
#endif
}

bool writeDataLoRa(byte* data, byte length)
{
#ifdef DEBUG
    Serial.println("Sending LoRa data: ");print(data, length);
#endif
#ifdef ROLE_GARAGE_CONTROLLER
    return mgrLoRa.sendtoWait(data, length, ADDRESS2);
#elif defined(ROLE_KEYFOB)
    return mgrLoRa.sendtoWait(data, length, ADDRESS1);
#endif
}

bool readDataSer(byte** data, byte& length)
{
    byte from;
    if (!mgrSer.available())
    {
        return false;
    }
    if(!mgrSer.recvfromAck(*data, &length, &from))
    {
        return false;
    }
#ifdef ROLE_GARAGE_CONTROLLER
    if(from != ADDRESS2)
    {
#ifdef DEBUG
        Serial.println("Sender doesn't match");
#endif
        return false;
    }
#elif defined(ROLE_KEYFOB)
    if(from != ADDRESS1)
    {
#ifdef DEBUG
        Serial.println("Sender doesn't match");
#endif
        return false;
    }
#endif
#ifdef DEBUG
    Serial.println("Received data: ");print(*data, length);
#endif
    return true;
}

bool readDataLoRa(byte** data, byte& length)
{
    byte from;
    if (!mgrLoRa.available())
    {
        return false;
    }
    if(!mgrLoRa.recvfromAck(*data, &length, &from))
    {
        return false;
    }
#ifdef ROLE_GARAGE_CONTROLLER
    if(from != ADDRESS2)
    {
#ifdef DEBUG
        Serial.println("Sender doesn't match");
#endif
        return false;
    }
#elif defined(ROLE_KEYFOB)
    if(from != ADDRESS1)
    {
#ifdef DEBUG
        Serial.println("Sender doesn't match");
#endif
        return false;
    }
#endif
    return true;
}

void dataReceived(byte* data, byte length)
{
    Serial.println("Event received with the following data:");
    print(data, length);
}

void setKeyInfo(byte* remoteId, byte length)
{
    Serial.println("Remote ID Event received with the following data:");
    print(remoteId, length);
    byte* key = cfg.findKey(remoteId, length);
    if(key)
    {
        k.setRemoteParty(remoteId, length, key);
    }
}

void print(const byte* array, byte length)
{
    Serial.print("Length = ");Serial.println(length,DEC);
    for (byte i = 0; i < length; i++)
    {
        Serial.print(array[i], HEX);
        Serial.print(" ");
        if ((i + 1) % 16 == 0)
        {
            Serial.println();
        }
    }
    Serial.println();
}
