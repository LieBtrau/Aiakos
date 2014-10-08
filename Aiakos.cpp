// Do not remove the include below
#include "Aiakos.h"
#include <RDM630/rdm630.h>

rdm630 rfid(2,3);
byte data[6];
byte length;

void setup()
{
    Serial.begin(9600);  // start serial to PC
    rfid.begin();
}


void loop()
{
    if(rfid.available()){
        rfid.getData(data,length);
        Serial.println("Data valid");
        for(int i=0;i<length;i++){
            Serial.print(data[i],HEX);
        }
        Serial.println();
    }
}
