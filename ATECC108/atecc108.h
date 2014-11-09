#ifndef ATECC108_H
#define ATECC108_H
#include "Arduino.h"
#include "ATECC108/ecc108_comm_marshaling.h"
class atecc108
{
public:
    atecc108();
    byte generateMac(const uint8_t *challenge, const word wKeyId, uint8_t* response_mac);
    byte checkmac(const uint8_t* challenge, uint8_t* response_mac, const word wKeyId);
    static const word ECC108_KEY_ID=0x0000;
    static const byte MAC_RSPSIZE=MAC_RSP_SIZE;
private:
    uint8_t ecc108e_wakeup_device();
    void ecc108e_sleep();
    uint8_t _device_id;
};

#endif // ATECC108_H
