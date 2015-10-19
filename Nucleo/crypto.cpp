#include "crypto.h"
#include "Base64.h"
#include "cmac.h"

/*
Private_key_0= "tv0BDCmcJYrEPzHrZ3cOjx1gFjxc7ZcS";
uint8_t private_0[NUM_ECC_DIGITS] = {0xB6, 0xFD, 0x01, 0x0C, 0x29, 0x9C, 0x25, 0x8A, 0xC4, 0x3F, 0x31, 0xEB, 0x67, 0x77, 0x0E, 0x8F, 0x1D, 0x60, 0x16, 0x3C, 0x5C, 0xED, 0x97, 0x12};
Public_key_0= "1sHy5tMrCRrG9SYs8x2ZbCQx0TiU7UYY0hgsWBetsJWnFeDlUKpytGbJLEvSmqGd";
uint8_t public_0[2*NUM_ECC_DIGITS] = {0xD6, 0xC1, 0xF2, 0xE6, 0xD3, 0x2B, 0x09, 0x1A, 0xC6, 0xF5, 0x26, 0x2C, 0xF3, 0x1D, 0x99, 0x6C, 0x24, 0x31, 0xD1, 0x38, 0x94, 0xED, 0x46, 0x180xD2, 0x18, 0x2C, 0x58, 0x17, 0xAD, 0xB0, 0x95, 0xA7, 0x15, 0xE0, 0xE5, 0x50, 0xAA, 0x72, 0xB4, 0x66, 0xC9, 0x2C, 0x4B, 0xD2, 0x9A, 0xA1, 0x9D};
*/
extern "C" { static int RNG(uint8_t *dest, unsigned size);}

Crypto::Crypto()
{
}

bool Crypto::setLocalKey(char *pLocalPrivateKey, char *pLocalPublicKey)
{
    if(base64_decode((char*)_pLocalPrivateKey, pLocalPrivateKey, (uECC_BYTES<<2)/3) != uECC_BYTES)
    {
        return false;
    }
    if(base64_decode((char*)_pLocalPublicKey, pLocalPublicKey, (uECC_BYTES<<3)/3) != uECC_BYTES*2)
    {
        return false;
    }
    return true;
}

void Crypto::cmacTest(){
    unsigned char T[BLOCK_SIZE];
    const unsigned char T64[BLOCK_SIZE] = { 0x51, 0xf0, 0xbe, 0xbf, 0x7e, 0x3b,
            0x9d, 0x92, 0xfc, 0x49, 0x74, 0x17, 0x79, 0x36, 0x3c, 0xfe };
    const unsigned char M[64] = { 0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f,
            0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a, 0xae, 0x2d,
            0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c, 0x9e, 0xb7, 0x6f, 0xac, 0x45,
            0xaf, 0x8e, 0x51, 0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11,
            0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef, 0xf6, 0x9f, 0x24,
            0x45, 0xdf, 0x4f, 0x9b, 0x17, 0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c,
            0x37, 0x10 };
    const unsigned char key[BLOCK_SIZE] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae,
            0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };

    const unsigned char key2[BLOCK_SIZE] = "password\0\0\0\0\0\0";
    const unsigned char M2[] = "1234567890;09876543";
    const unsigned char T2[BLOCK_SIZE] = { 0xef, 0xf2, 0x2d, 0x3a, 0x78, 0x7b,
            0xd8, 0xa5, 0x2b, 0xd4, 0x7e, 0xd5, 0x87, 0xd9, 0xb0, 0xd6 };

    Serial.print("--------------------------------------------------\n");
    Serial.print("K              ");
    printBuffer(key,BLOCK_SIZE);

    /*
     unsigned char L[BLOCK_SIZE], K1[BLOCK_SIZE], K2[BLOCK_SIZE];
     Serial.print("\nSubkey Generation\n"); //TODO: restore
     //AES_128(key, const_Zero, L);
     Serial.print("AES_128(key,0) ");
     print128(L);
     Serial.print("\n");
     generate_subkey(key, K1, K2);
     Serial.print("K1             ");
     print128(K1);
     Serial.print("\n");
     Serial.print("K2             ");
     print128(K2);
     Serial.print("\n");
     */

    Serial.print("\nExample 1: len = 0\n");
    Serial.print("M              ");
    Serial.print("<empty string>\n");

    AES_CMAC(key, M, 0, T);
    Serial.print("AES_CMAC       ");
    printBuffer(T,BLOCK_SIZE);

    Serial.print("\nExample 2: len = 16\n");
    Serial.print("M              ");
    printBuffer(M,16);
    AES_CMAC(key, M, 16, T);
    Serial.print("AES_CMAC       ");
    printBuffer(T,BLOCK_SIZE);
    Serial.print("\nExample 3: len = 40\n");
    Serial.print("M              ");
    printBuffer(M,40);
    AES_CMAC(key, M, 40, T);
    Serial.print("AES_CMAC       ");
    printBuffer(T,BLOCK_SIZE);

    Serial.print("\nExample 4: len = 64\n");
    Serial.print("M              ");
    printBuffer(M,64);
    AES_CMAC(key, M, 64, T);
    Serial.print("AES_CMAC       ");
    printBuffer(T,BLOCK_SIZE);

    Serial.print("AES_CMAC_CHECK: ");
    Serial.println(!AES_CMAC_CHECK(key, M, 64, T64) ? "OK" : "Failed");

    int ms2 = sizeof(M2) - 1;
    AES_CMAC(key2, M2, ms2, T);
    Serial.print("M2sz:");
    Serial.println(ms2);
    Serial.print("AES_CMAC       ");
    printBuffer(T,BLOCK_SIZE);
    Serial.print("AES_CMAC_CHECK: ");

    Serial.println(!AES_CMAC_CHECK(key2, M2, ms2, T2) ? "OK" : "Failed");
    Serial.print("--------------------------------------------------\n");
}

void Crypto::printBuffer(const byte* buf, byte len){
    for(int i=0;i<len;i++){
        Serial.print(buf[i], HEX);
        Serial.print(" ");
    }
    Serial.println("\r");
}

void Crypto::eccTest(){
//    uint8_t private1[uECC_BYTES];
//    uint8_t public1[uECC_BYTES * 2];
    uint8_t private2[uECC_BYTES];
    uint8_t public2[uECC_BYTES * 2];

    uint8_t secret1[uECC_BYTES];
    uint8_t secret2[uECC_BYTES];

    uECC_set_rng(&RNG);

    unsigned long a = millis();
//    uECC_make_key(public1, private1);
    unsigned long b = millis();
//    Serial.print("Made key 1 in "); Serial.println(b-a);

    a = millis();
    uECC_make_key(public2, private2);
    b = millis();
    Serial.print("Made key 2 in "); Serial.println(b-a);

    a = millis();
    int r = uECC_shared_secret(public2, _pLocalPrivateKey, secret1);
    b = millis();
    Serial.print("Shared secret 1 in "); Serial.println(b-a);
    if (!r) {
      Serial.print("shared_secret() failed (1)\n");
      return;
    }

    a = millis();
    r = uECC_shared_secret(_pLocalPublicKey, private2, secret2);
    b = millis();
    Serial.print("Shared secret 2 in "); Serial.println(b-a);
    if (!r) {
      Serial.print("shared_secret() failed (2)\n");
      return;
    }

    if (memcmp(secret1, secret2, sizeof(secret1)) != 0) {
      Serial.print("Shared secrets are not identical!\n");
    } else {
      Serial.print("Shared secrets are identical\n");
    }
}

extern "C" {

static int RNG(uint8_t *dest, unsigned size) {
  // Use the least-significant bits from the ADC for an unconnected pin (or connected to a source of
  // random noise). This can take a long time to generate random data if the result of analogRead(0)
  // doesn't change very frequently.
  while (size) {
    uint8_t val = 0;
    for (unsigned i = 0; i < 8; ++i) {
      int init = analogRead(0);
      int count = 0;
      while (analogRead(0) == init) {
        ++count;
      }

      if (count == 0) {
         val = (val << 1) | (init & 0x01);
      } else {
         val = (val << 1) | (count & 0x01);
      }
    }
    *dest = val;
    ++dest;
    --size;
  }
  // NOTE: it would be a good idea to hash the resulting random data using SHA-256 or similar.
  return 1;
}

}  // extern "C"

