#ifndef CRYPTOHELPER
#define CRYPTOHELPER

bool getSerialNumber(byte* bufout, byte length);
int RNG(uint8_t *dest, unsigned size);
void initRng();

#endif // CRYPTOHELPER

