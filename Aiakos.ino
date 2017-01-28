#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>

/* Connections to Nucleo F103RB
* XL1276   Protrinket 3V    Nucleo
* --------------------------------
* NSS      10               A2
* MOSI     11               D11
* MISO     12               D12
* SCK      13               D13  (Be careful, this is the 6th pin on the left row of the right most pin header connector, not the fifth!)
* REST     RST              NRST
* DIO0     3                D2
* VCC      3V               3V
* GND      G                GND
*
*/
#define SERVER

const byte CLIENT_ADDRESS=1;
const byte SERVER_ADDRESS=2;

#ifdef SERVER && !defined(CLIENT)
//Nucleo
RH_RF95 driver(A2,2);
uint8_t data[] = "And hello back to you";
RHReliableDatagram manager(driver, SERVER_ADDRESS);
#elif defined(CLIENT) && !defined(SERVER)
//ProTrinket
uint8_t data[] = "Hello World!";
RH_RF95 driver(10,3);
RHReliableDatagram manager(driver, CLIENT_ADDRESS);
#else
#error No device type defined.
#endif

void setup()
{
    Serial.begin(9600);
    while (!Serial) ; // Wait for serial port to be available
    if (!manager.init())
    {
        Serial.println("init failed");
    }
}

// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

void loop()
{
#ifdef SERVER
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
      }
    }
#elif defined(CLIENT)
  Serial.println("Sending to rf95_reliable_datagram_server");

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
      Serial.println("No reply, is rf95_reliable_datagram_server running?");
    }
  }
  else
    Serial.println("sendtoWait failed");
  delay(500);
#endif
}


