#include "EEPROMAnything.h"
#include "configuration.h"

#define DEBUG
#ifdef DEBUG
extern void print(const byte* array, byte length);
#endif

Configuration::Configuration()
{
}

void Configuration::initializeEEPROM(){
#ifdef ARDUINO_STM_NUCLEO_F103RB
    EEPROM.format();
#endif
    for(byte i=0;i<KEY_COUNT;i++)
    {
        _config.keys[i].keyValid=false;
    }
    saveData();
}

void Configuration::saveData(){
#ifdef DEBUG
    Serial.println("Saving data");
#endif
    print(_config.keys[0].sharedKey,16);
    EEPROM_writeAnything(0,_config);
}

bool Configuration::loadData(){
#ifdef DEBUG
    Serial.println("Loading data");
#endif
    bool bResult= EEPROM_readAnything(0,_config);
    print(_config.keys[0].sharedKey,KEY_SIZE);
    print(_config.keys[0].peerId,IDLENGTH);
    return bResult;
}

bool Configuration::init(){
    //Set up for STM32F103RB (see Arduino STM32 example code)
#ifdef ARDUINO_STM_NUCLEO_F103RB
    EEPROM.PageBase0 = 0x801F000;
    EEPROM.PageBase1 = 0x801F800;
    EEPROM.PageSize  = 0x400;
#endif
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

byte* Configuration::getId(byte index)
{
    return _config.keys[index].peerId;
}

