#ifndef CRYPTOHELPER
#define CRYPTOHELPER

int ATSHA_RNG(byte *dest, unsigned size);
bool getSerialNumber(byte* bufout, byte length);
int RNG(uint8_t *dest, unsigned size);

#endif // CRYPTOHELPER

