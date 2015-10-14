#include "crypto.h"
#include "Base64.h"

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

void Crypto::eccTest(){
//    uint8_t private1[uECC_BYTES];
    uint8_t private2[uECC_BYTES];

//    uint8_t public1[uECC_BYTES * 2];
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

