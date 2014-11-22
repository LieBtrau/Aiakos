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

uint8_t atecc108::signVerify(){
    ///** \brief This function serves as an example for verifying a signature by using
    // *         Sign and Verify commands.
    // *
    // *         First, Slot0 is configured to store the P256 private key. The SlotConfig0
    // *         is configured to 0x8F20, while KeyConfig is configured to 0x3300. Then
    // *         the Configuration need to be locked to enable GenKey command to write
    // *         the private key to the slot.
    // *         The next sequence is to generate a Signature using Sign command. The
    // *         message to be signed is passed to the device by using Nonce command in
    // *         passthrough mode.
    // *         The following sequence is to verify the signature using Verify command.
    // *         The message to be verified is passed to the device using Nonce command,
    // *         while the signature and the public key is passed as input of Verify
    // *         command.
    // * \return status of the operation
    // */
    // declared as "volatile" for easier debugging
    volatile uint8_t ret_code;

    // Make the command buffer the size of the Verify command.
    static uint8_t command[VERIFY_256_EXTERNAL_COUNT];

    // Random response buffer
    static uint8_t response_random[RANDOM_RSP_SIZE];

    // Make the response buffer the minimum size.
    static uint8_t response_status[ECC108_RSP_SIZE_MIN];

    // GenKey response buffer
    static uint8_t response_genkey[GENKEY_RSP_SIZE_LONG];

    // Dymmy random response buffer
    static uint8_t response_dummy_random[RANDOM_RSP_SIZE];

    // Sign response buffer
    static uint8_t response_sign[SIGN_RSP_SIZE];

    // Initialize the hardware interface.
    // Depending on which interface you have linked the
    // library to, it initializes SWI UART, SWI GPIO, or TWI.
    ecc108p_init();

#if defined(ECC108_EXAMPLE_ACTIVATE_GPIO_AUTH_MODE) && (defined(ECC108_SWI_UART) || defined(ECC108_SWI_BITBANG))
    // Set the GPIO in Authorization Output mode
    ret_code = ecc108e_activate_gpio_auth_mode(ECC108_SET_HIGH, ECC108_KEY_ID);
    if (ret_code != ECC108_SUCCESS) {
        if (ret_code == ECC108_FUNC_FAIL) {
            // The configuration zone has been locked.
            // Do nothing
        } else {
            return ret_code;
        }
    }
#endif

    // Check configuration of Slot0 to store private key for signing process.
    ret_code = ecc108e_check_private_key_slot0_config();
    if (ret_code != ECC108_SUCCESS) {
        return ret_code;
    }

    // Obtain random challenge from host device
    ret_code = ecc108e_wakeup_device();
    if (ret_code != ECC108_SUCCESS) {
        return ret_code;
    }

    ret_code = ecc108m_execute(ECC108_RANDOM, RANDOM_SEED_UPDATE, 0x0000, 0,
                               NULL, 0, NULL, 0, NULL, sizeof(command), command,
                               sizeof(response_random), response_random);

    // Put host device to sleep.
    ecc108p_sleep();
    if (ret_code != ECC108_SUCCESS) {
        return ret_code;
    }

    // Wake up client device
    ret_code = ecc108e_wakeup_device();
    if (ret_code != ECC108_SUCCESS) {
        return ret_code;
    }

    memset(response_genkey, 0, sizeof(response_genkey));
#ifdef ECC108_EXAMPLE_GENERATE_PRIVATE_KEY
    // Generate Private Key on slot0 using GenKey command with mode = 0x04.
    // This step is required if slot0 has not been programmed with private key.
    ret_code = ecc108m_execute(ECC108_GENKEY, GENKEY_MODE_PRIVATE,
                               ECC108_KEY_ID, 0, NULL, 0, NULL, 0, NULL, sizeof(command),
                               command, sizeof(response_genkey), response_genkey);
#else
    // Generate only public key from the existing private key
    ret_code = ecc108m_execute(ECC108_GENKEY, GENKEY_MODE_PUBLIC,
                               ECC108_KEY_ID, 0, NULL, 0, NULL, 0, NULL, sizeof(command),
                               command, sizeof(response_genkey), response_genkey);
#endif
    if (ret_code != ECC108_SUCCESS) {
        (void) ecc108p_sleep();
        return ret_code;
    }

    // Perform dummy random command for updating the random seed
    ret_code = ecc108m_execute(ECC108_RANDOM, RANDOM_SEED_UPDATE, 0x0000, 0,
                               NULL, 0, NULL, 0, NULL, sizeof(command), command,
                               sizeof(response_dummy_random), response_dummy_random);
    if (ret_code != ECC108_SUCCESS) {
        (void) ecc108p_sleep();
        return ret_code;
    }

    // Pass the message to be signed using Nonce command with mode = 0x03.
    memset(response_status, 0, sizeof(response_status));
    ret_code = ecc108m_execute(ECC108_NONCE, NONCE_MODE_PASSTHROUGH,
                               NONCE_MODE_RANDOM_OUT, NONCE_NUMIN_SIZE_PASSTHROUGH,
                               (uint8_t *) &response_random[ECC108_BUFFER_POS_DATA], 0, NULL,
                               0, NULL, sizeof(command), command, sizeof(response_status),
                               response_status);
    ret_code = ecc108e_check_response_status(ret_code, response_status);
    if (ret_code != ECC108_SUCCESS) {
        (void) ecc108p_sleep();
        return ret_code;
    }

    // Sign the message using Sign command with mode = 0x80.
    memset(response_sign, 0, sizeof(response_sign));
    ret_code = ecc108m_execute(ECC108_SIGN, SIGN_MODE_EXTERNAL, ECC108_KEY_ID,
                               0, NULL, 0, NULL, 0, NULL, sizeof(command), command,
                               sizeof(response_sign), response_sign);

    // Put client device to sleep.
    ecc108p_sleep();
    if (ret_code != ECC108_SUCCESS) {
        return ret_code;
    }

    // Now check the Signature using the Verify command.

    ret_code = ecc108e_wakeup_device();
    if (ret_code != ECC108_SUCCESS) {
        return ret_code;
    }

    // Pass the message which has been signed using Nonce command with mode = 0x03.
    memset(response_status, 0, sizeof(response_status));
    ret_code = ecc108m_execute(ECC108_NONCE, NONCE_MODE_PASSTHROUGH,
                               NONCE_MODE_RANDOM_OUT, NONCE_NUMIN_SIZE_PASSTHROUGH,
                               (uint8_t *) &response_random[ECC108_BUFFER_POS_DATA], 0, NULL,
                               0, NULL, sizeof(command), command, sizeof(response_status),
                               response_status);
    ret_code = ecc108e_check_response_status(ret_code, response_status);
    if (ret_code != ECC108_SUCCESS) {
        (void) ecc108p_sleep();
        return ret_code;
    }

    // Verify Signature by using Verify command with mode = 0x02.
    memset(response_status, 0, sizeof(response_status));
    ret_code = ecc108m_execute(ECC108_VERIFY, VERIFY_MODE_EXTERNAL,
                               VERIFY_KEY_P256, VERIFY_256_SIGNATURE_SIZE,
                               &response_sign[ECC108_BUFFER_POS_DATA], VERIFY_256_KEY_SIZE,
                               &response_genkey[ECC108_BUFFER_POS_DATA], 0, NULL,
                               sizeof(command), command, sizeof(response_status),
                               response_status);
    ret_code = ecc108e_check_response_status(ret_code, response_status);
    if (ret_code != ECC108_SUCCESS) {
        return ret_code;
    }

    // Put host device to sleep.
    ecc108p_sleep();

    return ret_code;
}

bool atecc108::getSerialNumber(byte* sn){
    const uint16_t SERIAL_PART1_ADDR = 0x00;
    const uint16_t SERIAL_PART2_ADDR = 0x02;
    const uint16_t SERIAL_PART3_ADDR = 0x03;

    byte retCode;
    uint8_t buf[ECC108_ZONE_ACCESS_4];
    if(sn==NULL){
        return false;
    }
    memset(sn,0,9);
    retCode=read4zonebytes(CONFIG_ZONE,SERIAL_PART1_ADDR, buf);
    if(retCode!=ECC108_SUCCESS){
        return false;
    }
    memcpy(sn,buf,sizeof(buf));
    retCode=read4zonebytes(CONFIG_ZONE,SERIAL_PART2_ADDR, buf);
    if(retCode!=ECC108_SUCCESS){
        return false;
    }
    memcpy(sn+4,buf,sizeof(buf));
    retCode=read4zonebytes(CONFIG_ZONE,SERIAL_PART3_ADDR, buf);
    if(retCode!=ECC108_SUCCESS){
        return false;
    }
    memcpy(sn+8,buf,1);
    return true;
}

uint8_t atecc108::checkmac(const byte* challenge, byte* response_mac, const word wKeyId)
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


byte atecc108::generateMac(const byte *challenge, const word wKeyId, byte *response_mac){
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

byte atecc108::read4zonebytes (ZONE zone, uint16_t addr, uint8_t *buf)
{
    uint8_t ret_code;
    // Make the command buffer the size of the Read command.
    uint8_t command[READ_COUNT];
    // Make the response buffer the size of the maximum Read response.
    uint8_t response[READ_32_RSP_SIZE];
    byte zoneParam;
    memset(buf,0,ECC108_ZONE_ACCESS_4);

    switch(zone){
    case CONFIG_ZONE:
        zoneParam=ECC108_ZONE_CONFIG;
        addr&=ECC108_ADDRESS_MASK_CONFIG;
        break;
    case OTP_ZONE:
        zoneParam=ECC108_ZONE_OTP;
        addr&=ECC108_ADDRESS_MASK_OTP;
        break;
    case DATA_ZONE:
        zoneParam=ECC108_ZONE_DATA;
        addr&=ECC108_ADDRESS_MASK;
        break;
    default:
        return false;
    }
    ret_code = ecc108e_wakeup_device();
    if (ret_code != ECC108_SUCCESS) {
        return ret_code;
    }
    ret_code = ecc108m_execute(ECC108_READ, zoneParam, addr,
                               0, NULL,
                               0, NULL,
                               0, NULL,
                               sizeof(command), command,
                               sizeof(response), response);
    if (ret_code != ECC108_SUCCESS) {
        return ret_code;
    }
    if (buf) {
        memcpy(buf, &response[ECC108_BUFFER_POS_DATA], ECC108_ZONE_ACCESS_4);
    }
    ecc108e_sleep();
    return ECC108_SUCCESS;
}

/** \brief This function checks the configuration of the device for verify external example.
 *
 *         SlotConfig0 to 0x8F20
 *           - ReadKey      0xF
 *           - NoMac          0
 *           - SingleUse      0
 *           - EncryptRead    0
 *           - IsSecret       1
 *           - WriteKey     0x0
 *           - WriteConfig  0x2
 *         KeyConfig0 to 0x3300
 *           - Private      1
 *           - PubInfo      1
 *           - KeyType  0b100
 *           - Lockable     1
 *           - ReqRandom    0
 *           - ReqAuth      0
 *           - AuthKey    0x0
 *           - RFU        0x0
 * \return status of the configuration
 */
uint8_t atecc108::ecc108e_check_private_key_slot0_config(void)
{
    // declared as "volatile" for easier debugging
    volatile uint8_t ret_code;

    // Slot configuration address for key (e.g. 48, 49)
    uint16_t slot_config_address = 20;

    const uint8_t read_config = 0x8F;
    const uint8_t write_config = 0x20;

    // Key configuration address for key (e.g. 48, 49)
    uint16_t key_config_address = 96;

    const uint8_t key_config_lsb = 0x33;
    const uint8_t key_config_msb = 0x00;

    // Make the command buffer the size of a Read command.
    uint8_t command[READ_COUNT];

    // Make the response buffer the minimum size of a Read response.
    uint8_t response[READ_4_RSP_SIZE];

    // Wake up the client device.
    ret_code = ecc108e_wakeup_device();
    if (ret_code != ECC108_SUCCESS) {
        return ret_code;
    }

    // Read device configuration of SlotConfig0.
    memset(response, 0, sizeof(response));
    ret_code = ecc108m_execute(ECC108_READ, ECC108_ZONE_CONFIG, slot_config_address >> 2, 0, NULL,
                               0, NULL, 0, NULL, sizeof(command), command, sizeof(response), response);
    if (ret_code != ECC108_SUCCESS) {
        return ret_code;
    }

    // Check the configuration of SlotConfig0.
    if (response[ECC108_BUFFER_POS_DATA] != read_config ||
            response[ECC108_BUFFER_POS_DATA + 1] != write_config) {
        // The Slot have not been configured correctly.
        // Throw error code.
        Serial.println("Slot config0 has wrong read or write config.");
        ecc108p_sleep();
        return ECC108_FUNC_FAIL;
    }

    // Read device configuration of KeyConfig0.
    memset(response, 0, sizeof(response));
    ret_code = ecc108m_execute(ECC108_READ, ECC108_ZONE_CONFIG, key_config_address >> 2, 0, NULL, 0, NULL,
                               0, NULL, sizeof(command), command, sizeof(response), response);
    if (ret_code != ECC108_SUCCESS) {
        return ret_code;
    }

    // Check the configuration of KeyConfig0.
    if (response[ECC108_BUFFER_POS_DATA] != key_config_lsb ||
            response[ECC108_BUFFER_POS_DATA + 1] != key_config_msb) {
        // The Key have not been configured correctly.
        // Throw error code.
        ecc108p_sleep();
        return ECC108_FUNC_FAIL;
    }

    // For this example, lock should be done by using ACES.
    // This function is only to show users how to lock the configuration zone
    // using a library function.
#if defined(ECC108_EXAMPLE_CONFIG_WITH_LOCK)
    ecc108p_sleep();
    if (ret_code != ECC108_SUCCESS) {
        return ret_code;
    }

    ret_code = ecc108e_lock_config_zone(ECC108_HOST_ADDRESS);
#endif

    // Check the configuration zone lock status
    ret_code = ecc108e_check_lock_status();

    return ret_code;
}

/** \brief This function checks the response status byte and puts the device
           to sleep if there was an error.
    \param[in] ret_code return code of function
    \param[in] response pointer to response buffer
    \return status of the operation
*/
uint8_t atecc108::ecc108e_check_response_status(uint8_t ret_code, uint8_t *response)
{
    if (ret_code != ECC108_SUCCESS) {
        ecc108p_sleep();
        return ret_code;
    }
    ret_code = response[ECC108_BUFFER_POS_STATUS];
    if (ret_code != ECC108_SUCCESS) {
        ecc108p_sleep();
    }

    return ret_code;
}

/** \brief This function checks the configuration lock status byte and puts the device
           to sleep if there was an error.
    \return status of the operation
*/
uint8_t atecc108::ecc108e_check_lock_status(void)
{
    // declared as "volatile" for easier debugging
    volatile uint8_t ret_code;

    uint16_t lock_config_address = 84;

    // Make the command buffer the size of a Read command.
    uint8_t command[READ_COUNT];

    // Make the response buffer the minimum size of a Read response.
    uint8_t response[READ_4_RSP_SIZE];

    // Make sure that configuration zone is locked.
    memset(response, 0, sizeof(response));
    ret_code = ecc108m_execute(ECC108_READ, ECC108_ZONE_CONFIG,
                lock_config_address >> 2, 0, NULL, 0, NULL,	0, NULL,
                sizeof(command), command, sizeof(response), response);
    if (ret_code != ECC108_SUCCESS) {
        return ret_code;
    }

    // Put client device to sleep.
    ecc108p_sleep();

    // Check the configuration lock status.
    if (response[4] == 0x55) {
        // Configuration Zone has not been locked.
        // Throw error code.
        return ECC108_FUNC_FAIL;
    }

    return ECC108_SUCCESS;
}
