/** \file
 *  \brief  Functions for I2C Physical Hardware Independent Layer of ECC108 Library
 *  \author Atmel Crypto Products
 *  \date   October 18, 2013

* \copyright Copyright (c) 2014 Atmel Corporation. All rights reserved.
*
* \atmel_crypto_device_library_license_start
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
* 3. The name of Atmel may not be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
* 4. This software may only be redistributed and used in connection with an
*    Atmel integrated circuit.
*
* THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
* EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
* OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
* ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
* \atmel_crypto_device_library_license_stop
 */
// Changes made: 2014, Christoph Tack

#define ECC108_GPIO_WAKEUP

#ifdef ECC108_GPIO_WAKEUP
#   include <avr/io.h>					//!< GPIO definitions
#endif

#ifdef ECC108_I2C_BITBANG
#   include "i2c_phys_bitbang.h"        // hardware dependent declarations for bit-banged I2C
#endif
#include "ecc108_config.h"
#include "ecc108_physical.h"            // declarations that are common to all interface implementations
#include "ecc108_lib_return_codes.h"    // declarations of function return codes
#include "timer_utilities.h"            // definitions for delay functions
#include "Arduino.h"
#include <Wire.h>

/** \defgroup ecc108_i2c Module 05: I2C Abstraction Module
 *
 * These functions and definitions abstract the I2C hardware. They implement the functions
 * declared in \ref ecc108_physical.h.
@{ */


/** \brief This enumeration lists all packet types sent to a ECC108 device.
 *
 * The following byte stream is sent to a ATECC108 I2C device:
 *    {I2C start} {I2C address} {word address} [{data}] {I2C stop}.
 * Data are only sent after a word address of value #ECC108_I2C_PACKET_FUNCTION_NORMAL.
 */
enum i2c_word_address {
    ECC108_I2C_PACKET_FUNCTION_RESET,  //!< Reset device.
    ECC108_I2C_PACKET_FUNCTION_SLEEP,  //!< Put device into Sleep mode.
    ECC108_I2C_PACKET_FUNCTION_IDLE,   //!< Put device into Idle mode.
    ECC108_I2C_PACKET_FUNCTION_NORMAL  //!< Write / evaluate data that follow this word address byte.
};


/** \brief This enumeration lists flags for I2C read or write addressing. */
enum i2c_read_write_flag {
    I2C_WRITE = (uint8_t) 0x00,  //!< write command flag
    I2C_READ  = (uint8_t) 0x01   //!< read command flag
};


//! I2C address is set when calling #ecc108p_init or #ecc108p_set_device_id.
static uint8_t device_address;


/** \brief This function sets the I2C address.
 *         Communication functions will use this address.
 *
 *  \param[in] id I2C address
 */
void ecc108p_set_device_id(uint8_t id)
{
    device_address = id>>1;
}



/** \brief This function initializes the hardware.
 */
void ecc108p_init(void)
{
    Wire.begin();
    device_address = ECC108_I2C_DEFAULT_ADDRESS;
}


/** \brief This function sends a I2C packet enclosed by
 *         a I2C start and stop to the device.
 *
This function combines a I2C packet send sequence that
is common to all packet types. Only if word_address is
#I2C_PACKET_FUNCTION_NORMAL, count and buffer parameters are
expected to be non-zero.
 * @param[in] word_address packet function code listed in #i2c_word_address
 * @param[in] count number of bytes in data buffer
 * @param[in] buffer pointer to data buffer
 * @return status of the operation
 */
static uint8_t ecc108p_i2c_send(uint8_t word_address, uint8_t count, uint8_t *buffer)
{
    Wire.beginTransmission(device_address);
    for(int i=0;i<count;i++)
    {
        Wire.write(buffer[i]);
    }
    return(Wire.endTransmission()!=0 ? ECC108_COMM_FAIL : ECC108_SUCCESS);
}


//#ifndef DEBUG_DIAMOND
//#   define DEBUG_DIAMOND
//#endif
/** \brief This I2C function generates a Wake-up pulse and delays.
 * \return status of the operation
 */
uint8_t ecc108p_wakeup(void)
{
    //If we take the ATSHA204 datasheet as a reference:
    //Wake the device by keeping SDA low for at least 60us.  SCL is ignored when the device is sleeping.
    //Wire library seems to work at 100kHz, so sending a 0x00 will keep SDA low long enough.
    Wire.beginTransmission(0);
    Wire.endTransmission();
    //After that, wait for 2.5ms
    delay(3);

    //to trigger the oscilloscope here with the external trigger
    pinMode(2,OUTPUT);
    digitalWrite(2,HIGH);
    digitalWrite(2,LOW);

    byte data=0;
    ecc108p_i2c_send(0,1,&data);
    return ECC108_COMM_FAIL;

    delay_10us(ECC108_WAKEUP_DELAY);

    return ECC108_SUCCESS;
}


/** \brief This function sends a command to the device.
 * \param[in] count number of bytes to send
 * \param[in] command pointer to command buffer
 * \return status of the operation
 */
uint8_t ecc108p_send_command(uint8_t count, uint8_t *command)
{
    return ecc108p_i2c_send(ECC108_I2C_PACKET_FUNCTION_NORMAL, count, command);
}


/** \brief This function puts the device into idle state.
 * \return status of the operation
 */
uint8_t ecc108p_idle(void)
{
    return ecc108p_i2c_send(ECC108_I2C_PACKET_FUNCTION_IDLE, 0, NULL);
}


/** \brief This function puts the device into low-power state.
 *  \return status of the operation
 */
uint8_t ecc108p_sleep(void)
{
    return ecc108p_i2c_send(ECC108_I2C_PACKET_FUNCTION_SLEEP, 0, NULL);
}


/** \brief This function resets the I/O buffer of the device.
 * \return status of the operation
 */
uint8_t ecc108p_reset_io(void)
{
    return ecc108p_i2c_send(ECC108_I2C_PACKET_FUNCTION_RESET, 0, NULL);
}


/** \brief This function receives a response from the device.
 *
 * \param[in] size size of rx buffer
 * \param[out] response pointer to rx buffer
 * \return status of the operation
 */
uint8_t ecc108p_receive_response(uint8_t size, uint8_t *response)
{
    uint8_t count,i=0;

    count=Wire.requestFrom(device_address, (uint8_t)256);
    if(count>0)
    {
        count--;
    }
    if ((count < ECC108_RSP_SIZE_MIN) || (count > size)) {
        return ECC108_INVALID_SIZE;
    }
    while(Wire.available())
    {
        response[ECC108_BUFFER_POS_DATA + i]=Wire.read();
        i++;
    }
    return ECC108_SUCCESS;
}


/** \brief This function resynchronizes communication.
 *
 * Parameters are not used for I2C.\n
 * Re-synchronizing communication is done in a maximum of three steps
 * listed below. This function implements the first step. Since
 * steps 2 and 3 (sending a Wake-up token and reading the response)
 * are the same for I2C and SWI, they are
 * implemented in the communication layer (#ecc108c_resync).
  <ol>
     <li>
       To ensure an IO channel reset, the system should send
       the standard I2C software reset sequence, as follows:
       <ul>
         <li>a Start condition</li>
         <li>nine cycles of SCL, with SDA held high</li>
         <li>another Start condition</li>
         <li>a Stop condition</li>
       </ul>
       It should then be possible to send a read sequence and
       if synchronization has completed properly the ATECC108 will
       acknowledge the device address. The chip may return data or
       may leave the bus floating (which the system will interpret
       as a data value of 0xFF) during the data periods.\n
       If the chip does acknowledge the device address, the system
       should reset the internal address counter to force the
       ATECC108 to ignore any partial input command that may have
       been sent. This can be accomplished by sending a write
       sequence to word address 0x00 (Reset), followed by a
       Stop condition.
     </li>
     <li>
       If the chip does NOT respond to the device address with an ACK,
       then it may be asleep. In this case, the system should send a
       complete Wake token and wait t_whi after the rising edge. The
       system may then send another read sequence and if synchronization
       has completed the chip will acknowledge the device address.
     </li>
     <li>
       If the chip still does not respond to the device address with
       an acknowledge, then it may be busy executing a command. The
       system should wait the longest TEXEC and then send the
       read sequence, which will be acknowledged by the chip.
     </li>
  </ol>
 * \param[in] size size of rx buffer
 * \param[out] response pointer to response buffer
 * \return status of the operation
 */
uint8_t ecc108p_resync(uint8_t size, uint8_t *response)
{
    uint8_t nine_clocks = 0xFF;
    Wire.requestFrom(nine_clocks,(uint8_t)0,(uint8_t)false);
    Wire.requestFrom(device_address,(uint8_t)0);

    // Try to send a Reset IO command if re-sync succeeded.
    return ecc108p_reset_io();
}

/** @} */
