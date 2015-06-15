/* -*- mode: c; c-file-style: "gnu" -*-
 * Copyright (C) 2013 Cryptotronix, LLC.
 *
 * This file is part of Hashlet.
 *
 * Hashlet is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * Hashlet is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Hashlet.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Modifications by Christoph Tack, 2014
 */

#include "hashlet.h"
#define VERBOSE_OUTPUT
Hashlet::Hashlet(DEVICE_TYPE dt): _sha204()
{
    switch(dt){
    case ATSHA204:
        _sha204.setAddress(0xC8);
        _dt=ATSHA204;
        break;
    case ATECC108:
        _sha204.setAddress(0xC0);
        _dt=ATECC108;
        break;
    default:
        _dt=UNDEFINED;
        break;
    }
}

void Hashlet::init(){
    _sha204.init();
}

//Code based on Cryptotronix hashlet (config_zone.*)
bool Hashlet::make_slot_config(uint8_t read_key, bool check_only,
                               bool single_use, bool encrypted_read,
                               bool is_secret, byte write_key,
                               bool derive_key, WRITECONFIG write_config,
                               uint8_t *slotConfig)
{
    const uint8_t WRITE_CONFIG_ALWAYS_MASK    =0b00000000;
    const uint8_t WRITE_CONFIG_NEVER_MASK     =0b10000000;
    const uint8_t WRITE_CONFIG_ENCRYPT_MASK   =0b01000000;
    const uint8_t WRITE_CONFIG_DERIVEKEY_MASK =0b00100000;
    const uint8_t CHECK_ONLY_MASK =    0b00010000;
    const uint8_t SINGLE_USE_MASK =    0b00100000;
    const uint8_t ENCRYPTED_READ_MASK= 0b01000000;
    const uint8_t IS_SECRET_MASK=      0b10000000;
    if(!slotConfig || read_key>15 || write_key>15 || !slotConfig){
        return false;
    }
    *slotConfig=read_key;
    if(check_only){
        *slotConfig |= CHECK_ONLY_MASK;
    }
    if(single_use){
        *slotConfig |= SINGLE_USE_MASK;
    }
    if(encrypted_read){
        *slotConfig |= ENCRYPTED_READ_MASK;
    }
    if(is_secret){
        *slotConfig |= IS_SECRET_MASK;
    }
    *(slotConfig+1)=write_key;
    if(derive_key){
        *(slotConfig+1) |= WRITE_CONFIG_DERIVEKEY_MASK;
    }
    switch(write_config){
    case NEVER:
        *(slotConfig+1) |= WRITE_CONFIG_NEVER_MASK;
        break;
    case ALWAYS:
        *(slotConfig+1) |= WRITE_CONFIG_ALWAYS_MASK;
        break;
    case ENCRYPT:
        *(slotConfig+1) |= WRITE_CONFIG_ENCRYPT_MASK;
        break;
    default:
        return false;
    }
    return true;
}

//see HashLet::config_zone.c
bool Hashlet::initialize(){
    uint8_t slotConfig[SHA204_ZONE_ACCESS_32];
    bool bResult;
    uint8_t ret_code=0;
    // Slots 0 -7 should be used for keyed hashed applications
    bResult=make_slot_config (0,     // Slot for Encrypted Reads
                              false, // Check Only
                              false, // Single Use
                              false, // Encrypted Read
                              true,  // Is secret
                              0,     // Slot for encrypted writes
                              false, // Derive Key
                              NEVER, //Write configuration
                              &slotConfig[0]);
    bResult|=make_slot_config (0, false, false, false, true, 0, true, NEVER, &slotConfig[2]);
    bResult|=make_slot_config (0, false, false, false, true, 0, false, NEVER, &slotConfig[4]);
    bResult|=make_slot_config (0, false, false, false, true, 0, true, NEVER, &slotConfig[6]);
    bResult|=make_slot_config (0, false, false, false, true, 0, false, NEVER, &slotConfig[8]);
    bResult|=make_slot_config (0, false, false, false, true, 0, true, NEVER, &slotConfig[10]);
    // Slots 6 -7 are key slots, to which the user can write and change the key.
    bResult|=make_slot_config (0, false, false, false, true, 6, false, ENCRYPT, &slotConfig[12]);
    bResult|=make_slot_config (0, false, false, false, true, 7, false, ENCRYPT, &slotConfig[14]);
    // Slots 8 - 11 Are reserved for password checking
    bResult|=make_slot_config (0, false, false, false, true, 0, false, NEVER, &slotConfig[16]);
    bResult|=make_slot_config (0, false, false, false, true, 0, false, NEVER, &slotConfig[18]);
    bResult|=make_slot_config (0, false, false, false, true, 0, false, NEVER, &slotConfig[20]);
    bResult|=make_slot_config (0, false, false, false, true, 0, false, NEVER, &slotConfig[22]);
    // Slots 12 - 13 should be used for user storage (freely RW)
    bResult|=make_slot_config (0, false, false, false, false, 0, false, ALWAYS, &slotConfig[24]);
    bResult|=make_slot_config (0, false, false, false, false, 0, false, ALWAYS, &slotConfig[26]);
    // Slots 14 and 15 are fixed test keys (they are not secret)
    bResult|=make_slot_config (0, false, false, false, false, 0, false, NEVER, &slotConfig[28]);
    bResult|=make_slot_config (0, false, false, false, false, 0, false, NEVER, &slotConfig[30]);
    if(!bResult){
        return false;
    }
#ifdef VERBOSE_OUTPUT
    Serial.print("Data slot configurations:");
    for(int i=0;i<32;i++){
        if(i%2==0){
            Serial.print("\r\n\tKey slot 0x");
            Serial.print(i>>1,HEX);
            Serial.print(": ");
        }
        Serial.print(slotConfig[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
#endif
    if(_sha204.sha204e_write_config_zone(slotConfig, SHA204::OTP_MODE_READONLY)!=SHA204_SUCCESS){
#ifdef VERBOSE_OUTPUT
        Serial.println("Can't write config zone.");
#endif
        return false;
    }
    switch(_dt){
    case UNDEFINED:
        return false;
    case ATSHA204:
         ret_code=_sha204.sha204e_lock_config_zone(SHA204_CONFIG_SIZE);
         break;
    case ATECC108:
        ret_code=_sha204.sha204e_lock_config_zone(ECC108_CONFIG_SIZE);
        break;
    }


    if(ret_code!=SHA204_SUCCESS){
#ifdef VERBOSE_OUTPUT
        Serial.print("Can't lock config zone: ");
        Serial.println(ret_code, HEX);
#endif
        return false;
    }
    return true;
}

/**
 *  \brief  ATSHA204 Helper Functions
 *  \author Atmel Crypto Products
 *  \date   January 15, 2013

 * \copyright Copyright (c) 2013 Atmel Corporation. All rights reserved.
 *
 * \atsha204_library_license_start
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
 * \atsha204_library_license_stop
 ** \brief This function creates a SHA256 digest on a little-endian system.
 *
 * Limitations: This function was implemented with the ATSHA204 CryptoAuth device
 * in mind. It will therefore only work for length values of len % 64 < 62.
 *
 * \param[in] len byte length of message
 * \param[in] message pointer to message
 * \param[out] digest SHA256 of message
 */
void sha204h_calculate_sha256(int32_t len, uint8_t *message, uint8_t *digest)
{
#define rotate_right(value, places) ((value >> places) | (value << (32 - places)))
#define SHA256_BLOCK_SIZE   (64)   // bytes
    int32_t j, swap_counter, len_mod = len % sizeof(int32_t);
    uint32_t i, w_index;
    int32_t message_index = 0;
    int32_t padded_len = len + 8; // 8 bytes for bit length
    uint32_t bit_len = len * 8;
    uint32_t s0, s1;
    uint32_t t1, t2;
    uint32_t maj, ch;
    uint32_t word_value;
    uint32_t rotate_register[8];

    union {
        uint32_t w_word[SHA256_BLOCK_SIZE];
        uint8_t w_byte[SHA256_BLOCK_SIZE * sizeof(int32_t)];
    } w_union;

    uint32_t hash[] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372,	0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };

    const uint32_t k[] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };

    // Process message.
    while (message_index <= padded_len) {

        // Break message into 64-byte blocks.
        w_index = 0;
        do {
            // Copy message chunk of four bytes (size of integer) into compression array.
            if (message_index < (len - len_mod)) {
                for (swap_counter = sizeof(int32_t) - 1; swap_counter >= 0; swap_counter--)
                    // No padding needed. Swap four message bytes to chunk array.
                    w_union.w_byte[swap_counter + w_index] = message[message_index++];

                w_index += sizeof(int32_t);
            }
            else {
                // We reached last complete word of message {len - (len mod 4)}.
                // Swap remaining bytes if any, append '1' bit and pad remaining
                // bytes of the last word.
                for (swap_counter = sizeof(int32_t) - 1;
                     swap_counter >= sizeof(int32_t) - len_mod; swap_counter--)
                    w_union.w_byte[swap_counter + w_index] = message[message_index++];
                w_union.w_byte[swap_counter + w_index] = 0x80;
                for (swap_counter--; swap_counter >= 0; swap_counter--)
                    w_union.w_byte[swap_counter + w_index] = 0;

                // Switch to word indexing.
                w_index += sizeof(int32_t);
                w_index /= sizeof(int32_t);

                // Pad last block with zeros to a block length % 56 = 0
                // and pad the four high bytes of "len" since we work only
                // with integers and not with long integers.
                while (w_index < 15)
                    w_union.w_word[w_index++] = 0;
                // Append original message length as 32-bit integer.
                w_union.w_word[w_index] = bit_len;
                // Indicate that the last block is being processed.
                message_index += SHA256_BLOCK_SIZE;
                // We are done with pre-processing last block.
                break;
            }
        } while (message_index % SHA256_BLOCK_SIZE);
        // Created one block.

        w_index = 16;
        while (w_index < SHA256_BLOCK_SIZE) {
            // right rotate for 32-bit variable in C: (value >> places) | (value << 32 - places)
            word_value = w_union.w_word[w_index - 15];
            s0 = rotate_right(word_value, 7) ^ rotate_right(word_value, 18) ^ (word_value >> 3);

            word_value = w_union.w_word[w_index - 2];
            s1 = rotate_right(word_value, 17) ^ rotate_right(word_value, 19) ^ (word_value >> 10);

            w_union.w_word[w_index] = w_union.w_word[w_index - 16] + s0 + w_union.w_word[w_index - 7] + s1;

            w_index++;
        }

        // Initialize hash value for this chunk.
        for (i = 0; i < 8; i++)
            rotate_register[i] = hash[i];

        // hash calculation loop
        for (i = 0; i < SHA256_BLOCK_SIZE; i++) {
            s0 = rotate_right(rotate_register[0], 2)
                    ^ rotate_right(rotate_register[0], 13)
                    ^ rotate_right(rotate_register[0], 22);
            maj = (rotate_register[0] & rotate_register[1])
                    ^ (rotate_register[0] & rotate_register[2])
                    ^ (rotate_register[1] & rotate_register[2]);
            t2 = s0 + maj;
            s1 = rotate_right(rotate_register[4], 6)
                    ^ rotate_right(rotate_register[4], 11)
                    ^ rotate_right(rotate_register[4], 25);
            ch =  (rotate_register[4] & rotate_register[5])
                    ^ (~rotate_register[4] & rotate_register[6]);
            t1 = rotate_register[7] + s1 + ch + k[i] + w_union.w_word[i];

            rotate_register[7] = rotate_register[6];
            rotate_register[6] = rotate_register[5];
            rotate_register[5] = rotate_register[4];
            rotate_register[4] = rotate_register[3] + t1;
            rotate_register[3] = rotate_register[2];
            rotate_register[2] = rotate_register[1];
            rotate_register[1] = rotate_register[0];
            rotate_register[0] = t1 + t2;
        }

        // Add the hash of this block to current result.
        for (i = 0; i < 8; i++)
            hash[i] += rotate_register[i];
    }

    // All blocks have been processed.
    // Concatenate the hashes to produce digest, MSB of every hash first.
    for (i = 0; i < 8; i++) {
        for (j = sizeof(int32_t) - 1; j >= 0; j--, hash[i] >>= 8)
            digest[i * sizeof(int32_t) + j] = hash[i] & 0xFF;
    }
}

bool Hashlet::showConfigZone(){
    byte config_data[ECC108_CONFIG_SIZE];
    byte ret_code=0;
    byte config_size=0;
    switch(_dt){
    case UNDEFINED:
        return false;
    case ATSHA204:
        config_size=SHA204_CONFIG_SIZE;
        break;
    case ATECC108:
        config_size=ECC108_CONFIG_SIZE;
        break;
    }
    ret_code=_sha204.sha204e_read_config_zone(config_data, config_size);

    if(ret_code!=SHA204_SUCCESS){
#ifdef VERBOSE_OUTPUT
        Serial.println("Can't read config zone.");
#endif
        return false;
    }
    Serial.println("Configuration Zone Data: ");
    for(int i=0;i<config_size;i++){
        if(i%4==0){
            Serial.print("\r\n\tAddress: 0x");
            Serial.print(i>>2, HEX);
            Serial.print(": ");
        }
        Serial.print(config_data[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
    return true;
}
