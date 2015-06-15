#ifndef HASHLET_H
#define HASHLET_H

#include "SHA204.h"
#include "SHA204Definitions.h"
#include "SHA204I2C.h"
#include "SHA204ReturnCodes.h"

class Hashlet
{
public:
    typedef enum{
        UNDEFINED,
        ATSHA204,
        ATECC108
    }DEVICE_TYPE;
    Hashlet(DEVICE_TYPE dt);
    void init();
    bool showConfigZone();
    bool initialize();
private:
    typedef enum{
        ALWAYS,
        NEVER,
        ENCRYPT
    } WRITECONFIG;
    bool make_slot_config(uint8_t read_key, bool check_only,
                          bool single_use, bool encrypted_read,
                          bool is_secret, byte write_key,
                          bool derive_key, WRITECONFIG write_config,
                          uint8_t* slotConfig);
    SHA204I2C _sha204;
    DEVICE_TYPE _dt;
};

#endif // HASHLET_H
