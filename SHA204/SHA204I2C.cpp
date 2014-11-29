/*
Copyright 2013 Nusku Networks

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

2014, Christoph Tack:
-removing receive_byte, receive_bytes, send_bytes
-new implemtation of many functions.
*/

#include "Arduino.h"
#include <Wire.h>

#ifdef TwoWire_h // Ensure this code only gets built if you have Wire.h included in the main sketch

#include "SHA204.h"
#include "SHA204ReturnCodes.h"
#include "SHA204Definitions.h"
#include "SHA204I2C.h"

uint16_t SHA204I2C::SHA204_RESPONSE_TIMEOUT() {
    return SHA204_RESPONSE_TIMEOUT_VALUE;
}

SHA204I2C::SHA204I2C(uint8_t deviceAddress) {
    _address = deviceAddress>>1;
}

void SHA204I2C::init() {
#ifdef TwoWire_h
    Wire.begin();
#endif
}

uint8_t SHA204I2C::chip_wakeup() {
    //If we take the ATSHA204 datasheet as a reference:
    //Wake the device by keeping SDA low for at least 60us.  SCL is ignored when the device is sleeping.
    //Wire library seems to work at 100kHz, so sending a 0x00 will keep SDA low long enough.
    Wire.beginTransmission(0);
    Wire.endTransmission();
    //Wait for the device to wake up
    delay(10 * SHA204_WAKEUP_DELAY);
    byte data=0;
    return send(0,1,&data);
}

uint8_t SHA204I2C::receive_response(uint8_t size, uint8_t *response) {
    uint8_t count;

    Wire.requestFrom(_address, (uint8_t)1,(uint8_t)false);
    if(!Wire.available())
    {
        Wire.requestFrom(_address,(uint8_t)0);//send a stop
        return SHA204_COMM_FAIL;
    }
    count=Wire.read();

    if ((count < SHA204_RSP_SIZE_MIN) || (count > size)) {
        return SHA204_INVALID_SIZE;
    }
    response[SHA204_BUFFER_POS_COUNT]=count;
    Wire.requestFrom(_address, (uint8_t)(count-1));
    for(uint8_t i=0;i<count-1;i++){
        if(!Wire.available()){
            return SHA204_INVALID_SIZE;
        }
        response[SHA204_BUFFER_POS_DATA + i]=Wire.read();
    }
    return SHA204_SUCCESS;
}

uint8_t SHA204I2C::send(uint8_t word_address, uint8_t count, uint8_t *buffer) {
    Wire.beginTransmission(_address);
    Wire.write(word_address);
    Wire.write(buffer,count);
    return(Wire.endTransmission()!=0 ? SHA204_COMM_FAIL : SHA204_SUCCESS);
}

uint8_t SHA204I2C::send_command(uint8_t count, uint8_t *command) {
    return send(SHA204_I2C_PACKET_FUNCTION_NORMAL, count, command);
}

uint8_t SHA204I2C::sha204p_sleep(void) {
    return send(SHA204_I2C_PACKET_FUNCTION_SLEEP, 0, NULL);
}

uint8_t SHA204I2C::resync(uint8_t size, uint8_t *response) {
    uint8_t nine_clocks = 0xFF;
    Wire.requestFrom(nine_clocks,(uint8_t)0,(uint8_t)false);
    Wire.requestFrom(_address,(uint8_t)0);

    // Try to send a Reset IO command if re-sync succeeded.
    int ret_code = reset_io();

    if (ret_code == SHA204_SUCCESS) {
        return ret_code;
    }

    // We lost communication. Send a Wake pulse and try
    // to receive a response (steps 2 and 3 of the
    // re-synchronization process).
    sha204p_sleep();
    ret_code = sha204c_wakeup();

    // Translate a return value of success into one
    // that indicates that the device had to be woken up
    // and might have lost its TempKey.
    return (ret_code == SHA204_SUCCESS ? SHA204_RESYNC_WITH_WAKEUP : ret_code);
}

uint8_t SHA204I2C::reset_io() {
    return send(SHA204_I2C_PACKET_FUNCTION_RESET, 0, NULL);
}

#endif
