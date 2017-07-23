#ifdef ARDUINO_SAM_DUE
//https://github.com/sebnil/DueFlashStorage/blob/master/examples/DueFlashStorageStructExample/DueFlashStorageStructExample.ino
#include <DueFlashStorage.h>
DueFlashStorage EEPROM;
#else
#include <EEPROM.h>
#endif

//#define DEBUG

#ifdef ARDUINO_ARCH_AVR
#include <util/crc16.h>
#else
//http://www.atmel.com/webdoc/AVRLibcReferenceManual/group__util__crc_1ga1c1d3ad875310cbc58000e24d981ad20.html
word _crc_ccitt_update (word crc, byte data)
{
    data ^=  crc & 0xFF;
    data ^= data << 4;

    return ((((uint16_t)data << 8) |  (crc>>8)) ^ (uint8_t)(data >> 4) ^ ((uint16_t)data << 3));
}
#endif

template <class T> int EEPROM_writeAnything(int ee, const T& value)
{
    word crc=0xFFFF;
    const byte* p = (const byte*)(const void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++){
        crc=_crc_ccitt_update(crc,*p);
        if(EEPROM.read(ee)!=*p)
        {
            EEPROM.write(ee, *p);
        }
        p++;
        ee++;
    }
    EEPROM.write(ee++, crc>>8);
    EEPROM.write(ee,crc & 0xFF);
    return i;
}

template <class T> boolean EEPROM_readAnything(int ee, T& value)
{
    word crcIn=0xFFFF;
    byte* p = (byte*)(void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++){
        *p = EEPROM.read(ee++);
        crcIn=_crc_ccitt_update(crcIn,*p);
        p++;
    }
    word crcRead=EEPROM.read(ee++)<<8;
    crcRead+=EEPROM.read(ee);
#ifdef DEBUG
    Serial.print("Read CRC: "); Serial.println(crcRead, HEX);
    Serial.print("Calculated CRC: "); Serial.println(crcIn, HEX);
#endif
    return (crcRead==crcIn?true:false);
}
