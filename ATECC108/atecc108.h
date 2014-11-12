#ifndef ATECC108_H
#define ATECC108_H
#include "Arduino.h"
#include "ATECC108/ecc108_comm_marshaling.h"
class atecc108
{
public:
    typedef enum
    {
        CONFIG_ZONE,
        OTP_ZONE,
        DATA_ZONE
    }ZONE;
    atecc108();
    byte generateMac(const byte *challenge, const word wKeyId, byte* response_mac);
    byte checkmac(const byte* challenge, byte* response_mac, const word wKeyId);
    bool getSerialNumber(byte* sn);
    static const word ECC108_KEY_ID=0x0000;
    static const byte MAC_RSPSIZE=MAC_RSP_SIZE;
private:
    //Get 72bit (=9byte) unique serial number
    byte read4zonebytes(ZONE zone, uint16_t addr, uint8_t *buf);
    uint8_t ecc108e_wakeup_device();
    void ecc108e_sleep();
    uint8_t _device_id;
};

#endif // ATECC108_H
