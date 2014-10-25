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

#include "Aiakos.h"

rdm630 rfid(6, 0);  //TX-pin of RDM630 connected to Arduino pin 6

void setup()
{
    Serial.begin(9600);  // start serial to PC
    rfid.begin();
    ecc108p_init();
    Serial.println(ecc108e_send_info_command());
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
