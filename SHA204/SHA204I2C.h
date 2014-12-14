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
*/

#include <Wire.h>

#ifdef TwoWire_h // Ensure this code only gets built if you have Wire.h included in the main sketch

#ifndef SHA204_Library_I2C_h
#define SHA204_Library_I2C_h

#include "SHA204.h"

#define I2C_CLOCK                         (400000.0)

#define I2C_FUNCTION_RETCODE_SUCCESS     ((uint8_t) 0x00) //!< Communication with device succeeded.
#define I2C_FUNCTION_RETCODE_COMM_FAIL   ((uint8_t) 0xF0) //!< Communication with device failed.
#define I2C_FUNCTION_RETCODE_TIMEOUT     ((uint8_t) 0xF1) //!< Communication timed out.
#define I2C_FUNCTION_RETCODE_NACK        ((uint8_t) 0xF8) //!< TWI nack

enum i2c_read_write_flag {
	I2C_WRITE = (uint8_t) 0x00,  //!< write command flag
	I2C_READ  = (uint8_t) 0x01   //!< read command flag
};

enum i2c_word_address {
	SHA204_I2C_PACKET_FUNCTION_RESET,  //!< Reset device.
	SHA204_I2C_PACKET_FUNCTION_SLEEP,  //!< Put device into Sleep mode.
	SHA204_I2C_PACKET_FUNCTION_IDLE,   //!< Put device into Idle mode.
	SHA204_I2C_PACKET_FUNCTION_NORMAL  //!< Write / evaluate data that follow this word address byte.
};

class SHA204I2C : public SHA204 {
public:
    void setAddress(uint8_t deviceAddress);
	void init();
private:
    const static uint16_t SHA204_RESPONSE_TIMEOUT_VALUE = ((uint16_t) 37);
    uint16_t SHA204_RESPONSE_TIMEOUT();

    uint8_t _address;
    uint8_t deviceAddress();
    uint8_t sha204p_sleep();
    uint8_t resync(uint8_t size, uint8_t *response);
    int start_operation(uint8_t readWrite);
    uint8_t receive_bytes(uint8_t count, uint8_t *data);
    uint8_t receive_byte(uint8_t *data);
    uint8_t send_bytes(uint8_t count, uint8_t *buffer);
    uint8_t send_byte(uint8_t value);
    uint8_t chip_wakeup();
    uint8_t reset_io();
    uint8_t receive_response(uint8_t size, uint8_t *response);
    uint8_t send(uint8_t word_address, uint8_t count, uint8_t *buffer);
    uint8_t send_command(uint8_t count, uint8_t * command);
};

#endif
#endif
