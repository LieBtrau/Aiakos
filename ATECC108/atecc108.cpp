#include "atecc108.h"
#include "ecc108_comm_marshaling.h"
#include "ecc108_lib_return_codes.h"

#   define ECC108_CLIENT_ADDRESS   (0xC0)
// To make the Mac / CheckMac examples work out-of-the-box, only one device is being
// used as example default. See above.
#   define ECC108_HOST_ADDRESS     (0xC0)


atecc108::atecc108():_device_id(ECC108_CLIENT_ADDRESS)
{
    // Initialize the hardware interface.
    // Depending on which interface you have linked the
    // library to, it initializes SWI UART, SWI GPIO, or TWI.
    ecc108p_init();
}

uint8_t atecc108::checkmac(const uint8_t* challenge, uint8_t *response_mac, const word wKeyId)
{
    uint8_t ret_code;
    // First four bytes of Mac command are needed for CheckMac command.
    uint8_t other_data[CHECKMAC_OTHER_DATA_SIZE];
    uint8_t command[CHECKMAC_COUNT];
    uint8_t response_checkmac[CHECKMAC_RSP_SIZE];

    ret_code = ecc108e_wakeup_device();
    if (ret_code != ECC108_SUCCESS)
    {
        return ret_code;
    }
    // CheckMac command with mode = 0.
    memset(response_checkmac, 0, sizeof(response_checkmac));
    // Copy Mac command byte 1 to 5 (op-code, param1, param2) to other_data.
    other_data[0]=ECC108_MAC;
    other_data[1]=MAC_MODE_CHALLENGE;
    other_data[2]=lowByte(wKeyId);
    other_data[3]=highByte(wKeyId);
    // Set the remaining nine bytes of other_data to 0.
    memset(&other_data[CHECKMAC_CLIENT_COMMAND_SIZE], 0, sizeof(other_data) - CHECKMAC_CLIENT_COMMAND_SIZE);
    //opcode(8b) = ECC108_CHECKMAC
    //param1(8b) = CHECKMAC_MODE_CHALLENGE,
    //param2(16b) = ECC108_KEY_ID
    //data1 (32bytes) = challenge
    //data2 (32bytes) = response_mac
    //data3 (13bytes) = other_data
    //TX-buffer(84bytes) = command
    //RX-buffer(4bytes) = response_checkmac
    ret_code = ecc108m_execute(ECC108_CHECKMAC, CHECKMAC_MODE_CHALLENGE, wKeyId,
                               MAC_CHALLENGE_SIZE, challenge,
                               CHECKMAC_CLIENT_RESPONSE_SIZE, &response_mac[ECC108_BUFFER_POS_DATA],
                               sizeof(other_data), other_data,
                               sizeof(command), command,
                               sizeof(response_checkmac), response_checkmac);
    if (ret_code != ECC108_SUCCESS) {
        return ret_code;
    }
    // Put host device to sleep.
    ecc108p_sleep();
    // Status byte = 0 means success. This line serves only a debug purpose.
    ret_code = response_checkmac[ECC108_BUFFER_POS_STATUS];
    return ret_code;
}


byte atecc108::generateMac(const uint8_t *challenge, const word wKeyId, uint8_t *response_mac){
    uint8_t ret_code;
    uint8_t command[CHECKMAC_COUNT];

    ret_code = ecc108e_wakeup_device();
    if (ret_code != ECC108_SUCCESS) {
        return ret_code;
    }
    // Mac command with mode = 0.
    memset(response_mac, 0, MAC_RSP_SIZE);

    ret_code = ecc108m_execute(ECC108_MAC, MAC_MODE_CHALLENGE, wKeyId,
                               MAC_CHALLENGE_SIZE, challenge,
                               0, NULL,
                               0, NULL,
                               sizeof(command), command,
                               MAC_RSP_SIZE, response_mac);
    if (ret_code != ECC108_SUCCESS) {
        return ret_code;
    }
    // Put client device to sleep.
    ecc108p_sleep();
    return ret_code;
}

/** \brief This function wakes up two I2C devices and puts one back to
           sleep, effectively waking up only one device among two that
           share SDA.
    \param[in] device_id which device to wake up
    \return status of the operation
*/
uint8_t atecc108::ecc108e_wakeup_device()
{
    uint8_t ret_code;
    uint8_t wakeup_response[ECC108_RSP_SIZE_MIN];
    memset(wakeup_response, 0, sizeof(wakeup_response));

    ecc108p_set_device_id(_device_id);

    // Wake up the devices.

    ret_code = ecc108c_wakeup(wakeup_response);
    if (ret_code != ECC108_SUCCESS) {
        ecc108e_sleep();
        return ret_code;
    }

#if defined(ECC108_I2C) && (ECC108_CLIENT_ADDRESS != ECC108_HOST_ADDRESS)
    // ECC108 I2C devices share SDA. We have to put the other device back to sleep.
    // Select other device...
    ecc108p_set_device_id(device_id == ECC108_CLIENT_ADDRESS ? ECC108_HOST_ADDRESS : ECC108_CLIENT_ADDRESS);
    // and put it to sleep.
    ret_code = ecc108p_sleep();
#endif
    return ret_code;
}

/**
 * \brief This function wraps \ref ecc108p_sleep().
 *        It puts both devices to sleep if two devices (client and host) are used.
 *        This function is also called when a Wakeup did not succeed.
 *        This would not make sense if a device did not wakeup and it is the only
 *        device on SDA, but if there are two devices (client and host) that
 *        share SDA, the device that is not selected might have woken up.
 */
void atecc108::ecc108e_sleep()
{
#if defined(ECC108_I2C) && (ECC108_CLIENT_ADDRESS != ECC108_HOST_ADDRESS)
    // Select host device...
    ecc108p_set_device_id(ECC108_HOST_ADDRESS);
    // and put it to sleep.
    (void) ecc108p_sleep();
    // Select client device...
    ecc108p_set_device_id(ECC108_CLIENT_ADDRESS);
    // and put it to sleep.
    (void) ecc108p_sleep();
#else
    (void) ecc108p_sleep();
#endif
}

