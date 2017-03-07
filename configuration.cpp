#include "EEPROMAnything.h"
#include "configuration.h"

#define DEBUG

Configuration::Configuration()
{
}

void Configuration::initializeEEPROM(){
    EEPROM.format();
    for(byte i=0;i<KEY_COUNT;i++)
    {
    _config.keys[i].keyValid=false;
    }
    saveData();
}

void Configuration::saveData(){
    EEPROM_writeAnything(0,_config);
}

bool Configuration::loadData(){
    return EEPROM_readAnything(0,_config);
}

bool Configuration::init(){
    //Set up for STM32F103RB (see Arduino STM32 example code)
    EEPROM.PageBase0 = 0x801F000;
    EEPROM.PageBase1 = 0x801F800;
    EEPROM.PageSize  = 0x400;
    //read EEPROM parameters
    if(!loadData()){
#ifdef DEBUG
        Serial.println("EEPROM data corrupt!");
#endif
        initializeEEPROM();
        return false;
    }
    return true;
}

void Configuration::setKey(byte index, const byte *id, const byte* key)
{
    memcpy(_config.keys[index].peerId, id, IDLENGTH);
    memcpy(_config.keys[index].sharedKey, key, KEY_SIZE);
    _config.keys[index].keyValid=true;
}

byte* Configuration::getKey(byte index)
{
    return _config.keys[index].sharedKey;
}
