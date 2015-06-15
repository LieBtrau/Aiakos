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

#ifndef SHA204_Library_SWI_h
#define SHA204_Library_SWI_h

#include "SHA204.h"

/* bitbang_config.h */

#define PORT_ACCESS_TIME  		(630)	//! time it takes to toggle the pin at CPU clock of 16 MHz (ns)
#define START_PULSE_WIDTH  		(4340)	//! width of start pulse (ns)
#define BIT_DELAY	      		(4)		//! delay macro for width of one pulse (start pulse or zero pulse, in ns)
#define RX_TX_DELAY        		(15)		//! turn around time when switching from receive to transmit
#define START_PULSE_TIME_OUT	(255)	//! This value is decremented while waiting for the falling edge of a start pulse.
#define ZERO_PULSE_TIME_OUT		(26)	//! This value is decremented while waiting for the falling edge of a zero pulse.

/* swi_phys.h */

#define SWI_FUNCTION_RETCODE_SUCCESS     ((uint8_t) 0x00) //!< Communication with device succeeded.
#define SWI_FUNCTION_RETCODE_TIMEOUT     ((uint8_t) 0xF1) //!< Communication timed out.
#define SWI_FUNCTION_RETCODE_RX_FAIL     ((uint8_t) 0xF9) //!< Communication failed after at least one byte was received.

/* sha204_swi.c */
#define SHA204_SWI_FLAG_CMD     ((uint8_t) 0x77) //!< flag preceding a command
#define SHA204_SWI_FLAG_TX      ((uint8_t) 0x88) //!< flag requesting a response
#define SHA204_SWI_FLAG_IDLE    ((uint8_t) 0xBB) //!< flag requesting to go into Idle mode
#define SHA204_SWI_FLAG_SLEEP   ((uint8_t) 0xCC) //!< flag requesting to go into Sleep mode

/* from sha204_config.h */
#define SWI_RECEIVE_TIME_OUT      ((uint16_t) 163)  //! #START_PULSE_TIME_OUT in us instead of loop counts
#define SWI_US_PER_BYTE           ((uint16_t) 313)  //! It takes 312.5 us to send a byte (9 single-wire bits / 230400 Baud * 8 flag bits).
#define SHA204_SYNC_TIMEOUT       ((uint8_t) 85)//! delay before sending a transmit flag in the synchronization routine

class SHA204SWI : public SHA204 {
private:
	const static uint16_t SHA204_RESPONSE_TIMEOUT_VALUE = ((uint16_t) SWI_RECEIVE_TIME_OUT + SWI_US_PER_BYTE);  //! SWI response timeout is the sum of receive timeout and the time it takes to send the TX flag.

	uint8_t device_pin;
	volatile uint8_t *device_port_DDR, *device_port_OUT, *device_port_IN;
	
	uint16_t SHA204_RESPONSE_TIMEOUT();
	void set_signal_pin(uint8_t is_high);
	uint8_t receive_bytes(uint8_t count, uint8_t *buffer);
	uint8_t send_bytes(uint8_t count, uint8_t *buffer);
	uint8_t send_byte(uint8_t value);
	uint8_t chip_wakeup();
	uint8_t receive_response(uint8_t size, uint8_t *response);
	uint8_t send_command(uint8_t count, uint8_t * command);

public:
	SHA204SWI(uint8_t pin);
	uint8_t sleep();
	uint8_t resync(uint8_t size, uint8_t *response);
};

#endif
