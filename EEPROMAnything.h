#ifdef ARDUINO_STM_NUCLEO_F103RB || defined(ARDUINO_ARCH_AVR)
#include <EEPROM.h>
#elif defined(ARDUINO_SAM_DUE)
//https://github.com/sebnil/DueFlashStorage/blob/master/examples/DueFlashStorageStructExample/DueFlashStorageStructExample.ino
#include <DueFlashStorage.h>
DueFlashStorage EEPROM;
#endif

#define DEBUG

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
#ifdef DEBUG
            Serial.print("Writing to EEPROM: address: ");
            Serial.print(ee, HEX);
            Serial.print(" value: ");
            Serial.println(*p);
#endif
            EEPROM.write(ee, *p++);
        }
        ee++;
    }
#ifdef DEBUG
            Serial.print("Writing CRC to EEPROM: address: ");
            Serial.print(ee, HEX);
            Serial.print(" value: ");
            Serial.println(crc>>8, HEX);
#endif
    EEPROM.write(ee++, crc>>8);
#ifdef DEBUG
            Serial.print("Writing CRC to EEPROM: address: ");
            Serial.print(ee, HEX);
            Serial.print(" value: ");
            Serial.println(crc & 0xFF, HEX);
#endif
    EEPROM.write(ee,crc & 0xFF);
    return i;
}

template <class T> boolean EEPROM_readAnything(int ee, T& value)
{
    word crcIn=0xFFFF;
    byte* p = (byte*)(void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++){
        *p++ = EEPROM.read(ee++);
#ifdef DEBUG
            Serial.print("Reading from EEPROM: address: ");
            Serial.print(ee-1, HEX);
            Serial.print(" value: ");
            Serial.println(*(p-1));
#endif
        crcIn=_crc_ccitt_update(crcIn,*(p-1));
    }
    word crcRead=EEPROM.read(ee++)<<8;
    crcRead+=EEPROM.read(ee);
#ifdef DEBUG
            Serial.print("Reading CRC: ");
            Serial.println(crcRead, HEX);
#endif
    return (crcRead==crcIn?true:false);
}
