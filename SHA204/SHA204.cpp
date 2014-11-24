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

#include "Arduino.h"
#include "SHA204.h"
#include "SHA204ReturnCodes.h"
#include "SHA204Definitions.h"

bool SHA204::serialNumber(uint8_t* sn) {
    uint8_t readCommand[READ_COUNT];
    uint8_t readResponse[READ_4_RSP_SIZE];

    if(sn==NULL){
        return false;
    }
    memset(sn,0,9);
    uint8_t retCode=read(readCommand, readResponse, SHA204_ZONE_CONFIG, ADDRESS_SN03);
    if(retCode!=SHA204_SUCCESS){
        return false;
    }
    memcpy(sn,&readResponse[SHA204_BUFFER_POS_DATA],4);
    retCode=read(readCommand, readResponse, SHA204_ZONE_CONFIG, ADDRESS_SN47);
    if(retCode!=SHA204_SUCCESS){
        return false;
    }
    memcpy(sn+4,&readResponse[SHA204_BUFFER_POS_DATA],4);
    retCode=read(readCommand, readResponse, SHA204_ZONE_CONFIG, ADDRESS_SN8);
    if(retCode!=SHA204_SUCCESS){
        return false;
    }
    memcpy(sn+8,&readResponse[SHA204_BUFFER_POS_DATA],1);
    return true;
}

/* Communication functions */

uint8_t SHA204::wakeup() {
    uint8_t ret_code = chip_wakeup();
    uint8_t response[SHA204_RSP_SIZE_MIN];
    if (ret_code != SHA204_SUCCESS)
        return ret_code;
    ret_code = receive_response(SHA204_RSP_SIZE_MIN, response);
    if (ret_code != SHA204_SUCCESS)
        return ret_code;

    // Verify status response.
    if (response[SHA204_BUFFER_POS_COUNT] != SHA204_RSP_SIZE_MIN)
        ret_code = SHA204_INVALID_SIZE;
    else if (response[SHA204_BUFFER_POS_STATUS] != SHA204_STATUS_BYTE_WAKEUP)
        ret_code = SHA204_COMM_FAIL;
    else
    {
        if ((response[SHA204_RSP_SIZE_MIN - SHA204_CRC_SIZE] != 0x33)
                || (response[SHA204_RSP_SIZE_MIN + 1 - SHA204_CRC_SIZE] != 0x43))
            ret_code = SHA204_BAD_CRC;
    }
    if (ret_code != SHA204_SUCCESS)
        delay(SHA204_COMMAND_EXEC_MAX);

    return ret_code;
}

uint8_t SHA204::send_and_receive(uint8_t *tx_buffer, uint8_t rx_size, uint8_t *rx_buffer, uint8_t execution_delay, uint8_t execution_timeout) {
    uint8_t ret_code = SHA204_FUNC_FAIL;
    uint8_t ret_code_resync;
    uint8_t n_retries_send;
    uint8_t n_retries_receive;
    uint8_t i;
    uint8_t status_byte;
    uint8_t count = tx_buffer[SHA204_BUFFER_POS_COUNT];
    uint8_t count_minus_crc = count - SHA204_CRC_SIZE;
    uint16_t execution_timeout_us = (uint16_t) (execution_timeout * 1000) + SHA204_RESPONSE_TIMEOUT();
    volatile uint16_t timeout_countdown;

    // Append CRC.
    calculate_crc(count_minus_crc, tx_buffer, tx_buffer + count_minus_crc);

    // Retry loop for sending a command and receiving a response.
    n_retries_send = SHA204_RETRY_COUNT + 1;

    while ((n_retries_send-- > 0) && (ret_code != SHA204_SUCCESS))
    {
        // Send command.
        ret_code = send_command(count, tx_buffer);
        if (ret_code != SHA204_SUCCESS)
        {
            if (resync(rx_size, rx_buffer) == SHA204_RX_NO_RESPONSE)
                return ret_code; // The device seems to be dead in the water.
            else
                continue;
        }

        // Wait minimum command execution time and then start polling for a response.
        delay(execution_delay);

        // Retry loop for receiving a response.
        n_retries_receive = SHA204_RETRY_COUNT + 1;
        while (n_retries_receive-- > 0)
        {
            // Reset response buffer.
            for (i = 0; i < rx_size; i++)
                rx_buffer[i] = 0;

            // Poll for response.
            timeout_countdown = execution_timeout_us;
            do
            {
                ret_code = receive_response(rx_size, rx_buffer);
                timeout_countdown -= SHA204_RESPONSE_TIMEOUT();
            }
            while ((timeout_countdown > SHA204_RESPONSE_TIMEOUT()) && (ret_code == SHA204_RX_NO_RESPONSE));

            if (ret_code == SHA204_RX_NO_RESPONSE)
            {
                // We did not receive a response. Re-synchronize and send command again.
                if (resync(rx_size, rx_buffer) == SHA204_RX_NO_RESPONSE)
                    // The device seems to be dead in the water.
                    return ret_code;
                else
                    break;
            }

            // Check whether we received a valid response.
            if (ret_code == SHA204_INVALID_SIZE)
            {
                // We see 0xFF for the count when communication got out of sync.
                ret_code_resync = resync(rx_size, rx_buffer);
                if (ret_code_resync == SHA204_SUCCESS)
                    // We did not have to wake up the device. Try receiving response again.
                    continue;
                if (ret_code_resync == SHA204_RESYNC_WITH_WAKEUP)
                    // We could re-synchronize, but only after waking up the device.
                    // Re-send command.
                    break;
                else
                    // We failed to re-synchronize.
                    return ret_code;
            }

            // We received a response of valid size.
            // Check the consistency of the response.
            ret_code = check_crc(rx_buffer);
            if (ret_code == SHA204_SUCCESS)
            {
                // Received valid response.
                if (rx_buffer[SHA204_BUFFER_POS_COUNT] > SHA204_RSP_SIZE_MIN)
                    // Received non-status response. We are done.
                    return ret_code;

                // Received status response.
                status_byte = rx_buffer[SHA204_BUFFER_POS_STATUS];

                // Translate the three possible device status error codes
                // into library return codes.
                if (status_byte == SHA204_STATUS_BYTE_PARSE)
                    return SHA204_PARSE_ERROR;
                if (status_byte == SHA204_STATUS_BYTE_EXEC)
                    return SHA204_CMD_FAIL;
                if (status_byte == SHA204_STATUS_BYTE_COMM)
                {
                    // In case of the device status byte indicating a communication
                    // error this function exits the retry loop for receiving a response
                    // and enters the overall retry loop
                    // (send command / receive response).
                    ret_code = SHA204_STATUS_CRC;
                    break;
                }

                // Received status response from CheckMAC, DeriveKey, GenDig,
                // Lock, Nonce, Pause, UpdateExtra, or Write command.
                return ret_code;
            }

            else
            {
                // Received response with incorrect CRC.
                ret_code_resync = resync(rx_size, rx_buffer);
                if (ret_code_resync == SHA204_SUCCESS)
                    // We did not have to wake up the device. Try receiving response again.
                    continue;
                if (ret_code_resync == SHA204_RESYNC_WITH_WAKEUP)
                    // We could re-synchronize, but only after waking up the device.
                    // Re-send command.
                    break;
                else
                    // We failed to re-synchronize.
                    return ret_code;
            } // block end of check response consistency

        } // block end of receive retry loop

    } // block end of send and receive retry loop

    return ret_code;
}


/* Marshaling functions */

uint8_t SHA204::random(uint8_t * tx_buffer, uint8_t * rx_buffer, uint8_t mode) {
    if (!tx_buffer || !rx_buffer || (mode > RANDOM_NO_SEED_UPDATE))
        return SHA204_BAD_PARAM;

    tx_buffer[SHA204_COUNT_IDX] = RANDOM_COUNT;
    tx_buffer[SHA204_OPCODE_IDX] = SHA204_RANDOM;
    tx_buffer[RANDOM_MODE_IDX] = mode & RANDOM_SEED_UPDATE;

    tx_buffer[RANDOM_PARAM2_IDX] =
            tx_buffer[RANDOM_PARAM2_IDX + 1] = 0;

    return send_and_receive(&tx_buffer[0], RANDOM_RSP_SIZE, &rx_buffer[0], RANDOM_DELAY, RANDOM_EXEC_MAX - RANDOM_DELAY);
}

uint8_t SHA204::dev_rev(uint8_t *tx_buffer, uint8_t *rx_buffer) {
    if (!tx_buffer || !rx_buffer)
        return SHA204_BAD_PARAM;

    tx_buffer[SHA204_COUNT_IDX] = DEVREV_COUNT;
    tx_buffer[SHA204_OPCODE_IDX] = SHA204_DEVREV;

    // Parameters are 0.
    tx_buffer[DEVREV_PARAM1_IDX] =
            tx_buffer[DEVREV_PARAM2_IDX] =
            tx_buffer[DEVREV_PARAM2_IDX + 1] = 0;

    return send_and_receive(&tx_buffer[0], DEVREV_RSP_SIZE, &rx_buffer[0],
            DEVREV_DELAY, DEVREV_EXEC_MAX - DEVREV_DELAY);
}

uint8_t SHA204::read(uint8_t *tx_buffer, uint8_t *rx_buffer, uint8_t zone, uint16_t address) {
    uint8_t rx_size;

    if (!tx_buffer || !rx_buffer || ((zone & ~READ_ZONE_MASK) != 0)
            || ((zone & READ_ZONE_MODE_32_BYTES) && (zone == SHA204_ZONE_OTP)))
        return SHA204_BAD_PARAM;

    address >>= 2;
    if ((zone & SHA204_ZONE_MASK) == SHA204_ZONE_CONFIG)
    {
        if (address > SHA204_ADDRESS_MASK_CONFIG)
            return SHA204_BAD_PARAM;
    }
    else if ((zone & SHA204_ZONE_MASK) == SHA204_ZONE_OTP)
    {
        if (address > SHA204_ADDRESS_MASK_OTP)
            return SHA204_BAD_PARAM;
    }
    else if ((zone & SHA204_ZONE_MASK) == SHA204_ZONE_DATA)
    {
        if (address > SHA204_ADDRESS_MASK)
            return SHA204_BAD_PARAM;
    }

    tx_buffer[SHA204_COUNT_IDX] = READ_COUNT;
    tx_buffer[SHA204_OPCODE_IDX] = SHA204_READ;
    tx_buffer[READ_ZONE_IDX] = zone;
    tx_buffer[READ_ADDR_IDX] = (uint8_t) (address & SHA204_ADDRESS_MASK);
    tx_buffer[READ_ADDR_IDX + 1] = 0;

    rx_size = (zone & SHA204_ZONE_COUNT_FLAG) ? READ_32_RSP_SIZE : READ_4_RSP_SIZE;

    return send_and_receive(&tx_buffer[0], rx_size, &rx_buffer[0], READ_DELAY, READ_EXEC_MAX - READ_DELAY);
}

uint8_t SHA204::execute(uint8_t op_code, uint8_t param1, uint16_t param2,
                        uint8_t datalen1, uint8_t *data1, uint8_t datalen2, uint8_t *data2, uint8_t datalen3, uint8_t *data3,
                        uint8_t tx_size, uint8_t *tx_buffer, uint8_t rx_size, uint8_t *rx_buffer) {
    uint8_t poll_delay, poll_timeout, response_size;
    uint8_t *p_buffer;
    uint8_t len;

    uint8_t ret_code = check_parameters(op_code, param1, param2,
                                        datalen1, data1, datalen2, data2, datalen3, data3,
                                        tx_size, tx_buffer, rx_size, rx_buffer);
    if (ret_code != SHA204_SUCCESS)
        return ret_code;

    // Supply delays and response size.
    switch (op_code)
    {
    case SHA204_CHECKMAC:
        poll_delay = CHECKMAC_DELAY;
        poll_timeout = CHECKMAC_EXEC_MAX - CHECKMAC_DELAY;
        response_size = CHECKMAC_RSP_SIZE;
        break;

    case SHA204_DERIVE_KEY:
        poll_delay = DERIVE_KEY_DELAY;
        poll_timeout = DERIVE_KEY_EXEC_MAX - DERIVE_KEY_DELAY;
        response_size = DERIVE_KEY_RSP_SIZE;
        break;

    case SHA204_DEVREV:
        poll_delay = DEVREV_DELAY;
        poll_timeout = DEVREV_EXEC_MAX - DEVREV_DELAY;
        response_size = DEVREV_RSP_SIZE;
        break;

    case SHA204_GENDIG:
        poll_delay = GENDIG_DELAY;
        poll_timeout = GENDIG_EXEC_MAX - GENDIG_DELAY;
        response_size = GENDIG_RSP_SIZE;
        break;

    case SHA204_HMAC:
        poll_delay = HMAC_DELAY;
        poll_timeout = HMAC_EXEC_MAX - HMAC_DELAY;
        response_size = HMAC_RSP_SIZE;
        break;

    case SHA204_LOCK:
        poll_delay = LOCK_DELAY;
        poll_timeout = LOCK_EXEC_MAX - LOCK_DELAY;
        response_size = LOCK_RSP_SIZE;
        break;

    case SHA204_MAC:
        poll_delay = MAC_DELAY;
        poll_timeout = MAC_EXEC_MAX - MAC_DELAY;
        response_size = MAC_RSP_SIZE;
        break;

    case SHA204_NONCE:
        poll_delay = NONCE_DELAY;
        poll_timeout = NONCE_EXEC_MAX - NONCE_DELAY;
        response_size = param1 == NONCE_MODE_PASSTHROUGH
                ? NONCE_RSP_SIZE_SHORT : NONCE_RSP_SIZE_LONG;
        break;

    case SHA204_PAUSE:
        poll_delay = PAUSE_DELAY;
        poll_timeout = PAUSE_EXEC_MAX - PAUSE_DELAY;
        response_size = PAUSE_RSP_SIZE;
        break;

    case SHA204_RANDOM:
        poll_delay = RANDOM_DELAY;
        poll_timeout = RANDOM_EXEC_MAX - RANDOM_DELAY;
        response_size = RANDOM_RSP_SIZE;
        break;

    case SHA204_READ:
        poll_delay = READ_DELAY;
        poll_timeout = READ_EXEC_MAX - READ_DELAY;
        response_size = (param1 & SHA204_ZONE_COUNT_FLAG)
                ? READ_32_RSP_SIZE : READ_4_RSP_SIZE;
        break;

    case SHA204_UPDATE_EXTRA:
        poll_delay = UPDATE_DELAY;
        poll_timeout = UPDATE_EXEC_MAX - UPDATE_DELAY;
        response_size = UPDATE_RSP_SIZE;
        break;

    case SHA204_WRITE:
        poll_delay = WRITE_DELAY;
        poll_timeout = WRITE_EXEC_MAX - WRITE_DELAY;
        response_size = WRITE_RSP_SIZE;
        break;

    default:
        poll_delay = 0;
        poll_timeout = SHA204_COMMAND_EXEC_MAX;
        response_size = rx_size;
    }

    // Assemble command.
    len = datalen1 + datalen2 + datalen3 + SHA204_CMD_SIZE_MIN;
    p_buffer = tx_buffer;
    *p_buffer++ = len;
    *p_buffer++ = op_code;
    *p_buffer++ = param1;
    *p_buffer++ = param2 & 0xFF;
    *p_buffer++ = param2 >> 8;

    if (datalen1 > 0) {
        memcpy(p_buffer, data1, datalen1);
        p_buffer += datalen1;
    }
    if (datalen2 > 0) {
        memcpy(p_buffer, data2, datalen2);
        p_buffer += datalen2;
    }
    if (datalen3 > 0) {
        memcpy(p_buffer, data3, datalen3);
        p_buffer += datalen3;
    }

    calculate_crc(len - SHA204_CRC_SIZE, tx_buffer, p_buffer);

    // Send command and receive response.
    return send_and_receive(&tx_buffer[0], response_size,
            &rx_buffer[0],	poll_delay, poll_timeout);
}

uint8_t SHA204::check_parameters(uint8_t op_code, uint8_t param1, uint16_t param2,
                                 uint8_t datalen1, uint8_t *data1, uint8_t datalen2, uint8_t *data2, uint8_t datalen3, uint8_t *data3,
                                 uint8_t tx_size, uint8_t *tx_buffer, uint8_t rx_size, uint8_t *rx_buffer) {
#ifdef SHA204_CHECK_PARAMETERS

    uint8_t len = datalen1 + datalen2 + datalen3 + SHA204_CMD_SIZE_MIN;
    if (!tx_buffer || tx_size < len || rx_size < SHA204_RSP_SIZE_MIN || !rx_buffer)
        return SHA204_BAD_PARAM;

    if ((datalen1 > 0 && !data1) || (datalen2 > 0 && !data2) || (datalen3 > 0 && !data3))
        return SHA204_BAD_PARAM;

    // Check parameters depending on op-code.
    switch (op_code)
    {
    case SHA204_CHECKMAC:
        if (
                // no null pointers allowed
                !data1 || !data2
                // No reserved bits should be set.
                || (param1 | CHECKMAC_MODE_MASK) != CHECKMAC_MODE_MASK
                // key_id > 15 not allowed
                || param2 > SHA204_KEY_ID_MAX
                )
            return SHA204_BAD_PARAM;
        break;

    case SHA204_DERIVE_KEY:
        if (param2 > SHA204_KEY_ID_MAX)
            return SHA204_BAD_PARAM;
        break;

    case SHA204_DEVREV:
        break;

    case SHA204_GENDIG:
        if ((param1 != GENDIG_ZONE_OTP) && (param1 != GENDIG_ZONE_DATA))
            return SHA204_BAD_PARAM;
        break;

    case SHA204_HMAC:
        if ((param1 & ~HMAC_MODE_MASK) != 0)
            return SHA204_BAD_PARAM;
        break;

    case SHA204_LOCK:
        if (((param1 & ~LOCK_ZONE_MASK) != 0)
                || ((param1 & LOCK_ZONE_NO_CRC) && (param2 != 0)))
            return SHA204_BAD_PARAM;
        break;

    case SHA204_MAC:
        if (((param1 & ~MAC_MODE_MASK) != 0)
                || (((param1 & MAC_MODE_BLOCK2_TEMPKEY) == 0) && !data1))
            return SHA204_BAD_PARAM;
        break;

    case SHA204_NONCE:
        if (  !data1
              || (param1 > NONCE_MODE_PASSTHROUGH)
              || (param1 == NONCE_MODE_INVALID)
              )
            return SHA204_BAD_PARAM;
        break;

    case SHA204_PAUSE:
        break;

    case SHA204_RANDOM:
        if (param1 > RANDOM_NO_SEED_UPDATE)
            return SHA204_BAD_PARAM;
        break;

    case SHA204_READ:
        if (((param1 & ~READ_ZONE_MASK) != 0)
                || ((param1 & READ_ZONE_MODE_32_BYTES) && (param1 == SHA204_ZONE_OTP)))
            return SHA204_BAD_PARAM;
        break;

    case SHA204_TEMPSENSE:
        break;

    case SHA204_UPDATE_EXTRA:
        if (param1 > UPDATE_CONFIG_BYTE_86)
            return SHA204_BAD_PARAM;
        break;

    case SHA204_WRITE:
        if (!data1 || ((param1 & ~WRITE_ZONE_MASK) != 0))
            return SHA204_BAD_PARAM;
        break;

    default:
        // unknown op-code
        return SHA204_BAD_PARAM;
    }

    return SHA204_SUCCESS;

#else
    return SHA204_SUCCESS;
#endif
}

/* CRC Calculator and Checker */

void SHA204::calculate_crc(uint8_t length, uint8_t *data, uint8_t *crc)  {
    uint8_t counter;
    uint16_t crc_register = 0;
    uint16_t polynom = 0x8005;
    uint8_t shift_register;
    uint8_t data_bit, crc_bit;

    for (counter = 0; counter < length; counter++)
    {
        for (shift_register = 0x01; shift_register > 0x00; shift_register <<= 1)
        {
            data_bit = (data[counter] & shift_register) ? 1 : 0;
            crc_bit = crc_register >> 15;

            // Shift CRC to the left by 1.
            crc_register <<= 1;

            if ((data_bit ^ crc_bit) != 0)
                crc_register ^= polynom;
        }
    }
    crc[0] = (uint8_t) (crc_register & 0x00FF);
    crc[1] = (uint8_t) (crc_register >> 8);
}

uint8_t SHA204::check_crc(uint8_t *response) {
    uint8_t crc[SHA204_CRC_SIZE];
    uint8_t count = response[SHA204_BUFFER_POS_COUNT];

    count -= SHA204_CRC_SIZE;
    calculate_crc(count, response, crc);

    return (crc[0] == response[count] && crc[1] == response[count + 1])
            ? SHA204_SUCCESS : SHA204_BAD_CRC;
}

uint8_t SHA204::check_mac(uint8_t *tx_buffer, uint8_t *rx_buffer,
                          uint8_t mode, uint8_t key_id, uint8_t *client_challenge, uint8_t *client_response, uint8_t *other_data) {
    if (
            // no null pointers allowed
            !tx_buffer || !rx_buffer || !client_response || !other_data
            // No reserved bits should be set.
            || (mode | CHECKMAC_MODE_MASK) != CHECKMAC_MODE_MASK
            // key_id > 15 not allowed
            || key_id > SHA204_KEY_ID_MAX
            )
        return SHA204_BAD_PARAM;

    tx_buffer[SHA204_COUNT_IDX] = CHECKMAC_COUNT;
    tx_buffer[SHA204_OPCODE_IDX] = SHA204_CHECKMAC;
    tx_buffer[CHECKMAC_MODE_IDX] = mode & CHECKMAC_MODE_MASK;
    tx_buffer[CHECKMAC_KEYID_IDX]= key_id;
    tx_buffer[CHECKMAC_KEYID_IDX + 1] = 0;
    if (client_challenge == NULL)
        memset(&tx_buffer[CHECKMAC_CLIENT_CHALLENGE_IDX], 0, CHECKMAC_CLIENT_CHALLENGE_SIZE);
    else
        memcpy(&tx_buffer[CHECKMAC_CLIENT_CHALLENGE_IDX], client_challenge, CHECKMAC_CLIENT_CHALLENGE_SIZE);

    memcpy(&tx_buffer[CHECKMAC_CLIENT_RESPONSE_IDX], client_response, CHECKMAC_CLIENT_RESPONSE_SIZE);
    memcpy(&tx_buffer[CHECKMAC_DATA_IDX], other_data, CHECKMAC_OTHER_DATA_SIZE);

    return send_and_receive(&tx_buffer[0], CHECKMAC_RSP_SIZE, &rx_buffer[0],
            CHECKMAC_DELAY, CHECKMAC_EXEC_MAX - CHECKMAC_DELAY);
}

uint8_t SHA204::derive_key(uint8_t *tx_buffer, uint8_t *rx_buffer,
                           uint8_t random, uint8_t target_key, uint8_t *mac) {
    if (!tx_buffer || !rx_buffer || ((random & ~DERIVE_KEY_RANDOM_FLAG) != 0)
            || (target_key > SHA204_KEY_ID_MAX))
        return SHA204_BAD_PARAM;

    tx_buffer[SHA204_OPCODE_IDX] = SHA204_DERIVE_KEY;
    tx_buffer[DERIVE_KEY_RANDOM_IDX] = random;
    tx_buffer[DERIVE_KEY_TARGETKEY_IDX] = target_key;
    tx_buffer[DERIVE_KEY_TARGETKEY_IDX + 1] = 0;
    if (mac != NULL)
    {
        memcpy(&tx_buffer[DERIVE_KEY_MAC_IDX], mac, DERIVE_KEY_MAC_SIZE);
        tx_buffer[SHA204_COUNT_IDX] = DERIVE_KEY_COUNT_LARGE;
    }
    else
        tx_buffer[SHA204_COUNT_IDX] = DERIVE_KEY_COUNT_SMALL;

    return send_and_receive(&tx_buffer[0], DERIVE_KEY_RSP_SIZE, &rx_buffer[0],
            DERIVE_KEY_DELAY, DERIVE_KEY_EXEC_MAX - DERIVE_KEY_DELAY);
}

uint8_t SHA204::gen_dig(uint8_t *tx_buffer, uint8_t *rx_buffer,
                        uint8_t zone, uint8_t key_id, uint8_t *other_data) {
    if (!tx_buffer || !rx_buffer || (zone > GENDIG_ZONE_DATA))
        return SHA204_BAD_PARAM;

    if (((zone == GENDIG_ZONE_OTP) && (key_id > SHA204_OTP_BLOCK_MAX))
            || ((zone == GENDIG_ZONE_DATA) && (key_id > SHA204_KEY_ID_MAX)))
        return SHA204_BAD_PARAM;

    tx_buffer[SHA204_OPCODE_IDX] = SHA204_GENDIG;
    tx_buffer[GENDIG_ZONE_IDX] = zone;
    tx_buffer[GENDIG_KEYID_IDX] = key_id;
    tx_buffer[GENDIG_KEYID_IDX + 1] = 0;
    if (other_data != NULL)
    {
        memcpy(&tx_buffer[GENDIG_DATA_IDX], other_data, GENDIG_OTHER_DATA_SIZE);
        tx_buffer[SHA204_COUNT_IDX] = GENDIG_COUNT_DATA;
    }
    else
        tx_buffer[SHA204_COUNT_IDX] = GENDIG_COUNT;

    return send_and_receive(&tx_buffer[0], GENDIG_RSP_SIZE, &rx_buffer[0],
            GENDIG_DELAY, GENDIG_EXEC_MAX - GENDIG_DELAY);

}

uint8_t SHA204::hmac(uint8_t *tx_buffer, uint8_t *rx_buffer, uint8_t mode, uint16_t key_id) {
    if (!tx_buffer || !rx_buffer || ((mode & ~HMAC_MODE_MASK) != 0))
        return SHA204_BAD_PARAM;

    tx_buffer[SHA204_COUNT_IDX] = HMAC_COUNT;
    tx_buffer[SHA204_OPCODE_IDX] = SHA204_HMAC;
    tx_buffer[HMAC_MODE_IDX] = mode;

    // Although valid key identifiers are only
    // from 0 to 15, all 16 bits are used in the HMAC message.
    tx_buffer[HMAC_KEYID_IDX] = key_id & 0xFF;
    tx_buffer[HMAC_KEYID_IDX + 1] = key_id >> 8;

    return send_and_receive(&tx_buffer[0], HMAC_RSP_SIZE, &rx_buffer[0],
            HMAC_DELAY, HMAC_EXEC_MAX - HMAC_DELAY);
}

uint8_t SHA204::lock(uint8_t *tx_buffer, uint8_t *rx_buffer, uint8_t zone, uint16_t summary) {
    if (!tx_buffer || !rx_buffer || ((zone & ~LOCK_ZONE_MASK) != 0)
            || ((zone & LOCK_ZONE_NO_CRC) && (summary != 0)))
        return SHA204_BAD_PARAM;

    tx_buffer[SHA204_COUNT_IDX] = LOCK_COUNT;
    tx_buffer[SHA204_OPCODE_IDX] = SHA204_LOCK;
    tx_buffer[LOCK_ZONE_IDX] = zone & LOCK_ZONE_MASK;
    tx_buffer[LOCK_SUMMARY_IDX]= summary & 0xFF;
    tx_buffer[LOCK_SUMMARY_IDX + 1]= summary >> 8;
    return send_and_receive(&tx_buffer[0], LOCK_RSP_SIZE, &rx_buffer[0],
            LOCK_DELAY, LOCK_EXEC_MAX - LOCK_DELAY);
}

uint8_t SHA204::mac(uint8_t *tx_buffer, uint8_t *rx_buffer,
                    uint8_t mode, uint16_t key_id, uint8_t *challenge) {
    if (!tx_buffer || !rx_buffer || ((mode & ~MAC_MODE_MASK) != 0)
            || (((mode & MAC_MODE_BLOCK2_TEMPKEY) == 0) && !challenge))
        return SHA204_BAD_PARAM;

    tx_buffer[SHA204_COUNT_IDX] = MAC_COUNT_SHORT;
    tx_buffer[SHA204_OPCODE_IDX] = SHA204_MAC;
    tx_buffer[MAC_MODE_IDX] = mode;
    tx_buffer[MAC_KEYID_IDX] = key_id & 0xFF;
    tx_buffer[MAC_KEYID_IDX + 1] = key_id >> 8;
    if ((mode & MAC_MODE_BLOCK2_TEMPKEY) == 0)
    {
        memcpy(&tx_buffer[MAC_CHALLENGE_IDX], challenge, MAC_CHALLENGE_SIZE);
        tx_buffer[SHA204_COUNT_IDX] = MAC_COUNT_LONG;
    }

    return send_and_receive(&tx_buffer[0], MAC_RSP_SIZE, &rx_buffer[0],
            MAC_DELAY, MAC_EXEC_MAX - MAC_DELAY);
}

uint8_t SHA204::nonce(uint8_t *tx_buffer, uint8_t *rx_buffer, uint8_t mode, uint8_t *numin) {
    uint8_t rx_size;

    if (!tx_buffer || !rx_buffer || !numin
            || (mode > NONCE_MODE_PASSTHROUGH) || (mode == NONCE_MODE_INVALID))
        return SHA204_BAD_PARAM;

    tx_buffer[SHA204_OPCODE_IDX] = SHA204_NONCE;
    tx_buffer[NONCE_MODE_IDX] = mode;

    // 2. parameter is 0.
    tx_buffer[NONCE_PARAM2_IDX] =
            tx_buffer[NONCE_PARAM2_IDX + 1] = 0;

    if (mode != NONCE_MODE_PASSTHROUGH)
    {
        memcpy(&tx_buffer[NONCE_INPUT_IDX], numin, NONCE_NUMIN_SIZE);
        tx_buffer[SHA204_COUNT_IDX] = NONCE_COUNT_SHORT;
        rx_size = NONCE_RSP_SIZE_LONG;
    }
    else
    {
        memcpy(&tx_buffer[NONCE_INPUT_IDX], numin, NONCE_NUMIN_SIZE_PASSTHROUGH);
        tx_buffer[SHA204_COUNT_IDX] = NONCE_COUNT_LONG;
        rx_size = NONCE_RSP_SIZE_SHORT;
    }

    return send_and_receive(&tx_buffer[0], rx_size, &rx_buffer[0],
            NONCE_DELAY, NONCE_EXEC_MAX - NONCE_DELAY);
}

uint8_t SHA204::pause(uint8_t *tx_buffer, uint8_t *rx_buffer, uint8_t selector) {
    if (!tx_buffer || !rx_buffer)
        return SHA204_BAD_PARAM;

    tx_buffer[SHA204_COUNT_IDX] = PAUSE_COUNT;
    tx_buffer[SHA204_OPCODE_IDX] = SHA204_PAUSE;
    tx_buffer[PAUSE_SELECT_IDX] = selector;

    // 2. parameter is 0.
    tx_buffer[PAUSE_PARAM2_IDX] =
            tx_buffer[PAUSE_PARAM2_IDX + 1] = 0;

    return send_and_receive(&tx_buffer[0], PAUSE_RSP_SIZE, &rx_buffer[0],
            PAUSE_DELAY, PAUSE_EXEC_MAX - PAUSE_DELAY);
}

uint8_t SHA204::update_extra(uint8_t *tx_buffer, uint8_t *rx_buffer, uint8_t mode, uint8_t new_value) {
    if (!tx_buffer || !rx_buffer || (mode > UPDATE_CONFIG_BYTE_86))
        return SHA204_BAD_PARAM;

    tx_buffer[SHA204_COUNT_IDX] = UPDATE_COUNT;
    tx_buffer[SHA204_OPCODE_IDX] = SHA204_UPDATE_EXTRA;
    tx_buffer[UPDATE_MODE_IDX] = mode;
    tx_buffer[UPDATE_VALUE_IDX] = new_value;
    tx_buffer[UPDATE_VALUE_IDX + 1] = 0;

    return send_and_receive(&tx_buffer[0], UPDATE_RSP_SIZE, &rx_buffer[0],
            UPDATE_DELAY, UPDATE_EXEC_MAX - UPDATE_DELAY);
}

uint8_t SHA204::write(uint8_t *tx_buffer, uint8_t *rx_buffer,
                      uint8_t zone, uint16_t address, uint8_t *new_value, uint8_t *mac) {
    uint8_t *p_command;
    uint8_t count;

    if (!tx_buffer || !rx_buffer || !new_value || ((zone & ~WRITE_ZONE_MASK) != 0))
        return SHA204_BAD_PARAM;

    address >>= 2;
    if ((zone & SHA204_ZONE_MASK) == SHA204_ZONE_CONFIG) {
        if (address > SHA204_ADDRESS_MASK_CONFIG)
            return SHA204_BAD_PARAM;
    }
    else if ((zone & SHA204_ZONE_MASK) == SHA204_ZONE_OTP) {
        if (address > SHA204_ADDRESS_MASK_OTP)
            return SHA204_BAD_PARAM;
    }
    else if ((zone & SHA204_ZONE_MASK) == SHA204_ZONE_DATA) {
        if (address > SHA204_ADDRESS_MASK)
            return SHA204_BAD_PARAM;
    }

    p_command = &tx_buffer[SHA204_OPCODE_IDX];
    *p_command++ = SHA204_WRITE;
    *p_command++ = zone;
    *p_command++ = (uint8_t) (address & SHA204_ADDRESS_MASK);
    *p_command++ = 0;

    count = (zone & SHA204_ZONE_COUNT_FLAG) ? SHA204_ZONE_ACCESS_32 : SHA204_ZONE_ACCESS_4;
    memcpy(p_command, new_value, count);
    p_command += count;

    if (mac != NULL)
    {
        memcpy(p_command, mac, WRITE_MAC_SIZE);
        p_command += WRITE_MAC_SIZE;
    }

    // Supply count.
    tx_buffer[SHA204_COUNT_IDX] = (uint8_t) (p_command - &tx_buffer[0] + SHA204_CRC_SIZE);

    return send_and_receive(&tx_buffer[0], WRITE_RSP_SIZE, &rx_buffer[0],
            WRITE_DELAY, WRITE_EXEC_MAX - WRITE_DELAY);
}
