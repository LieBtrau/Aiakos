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

bool Configuration::loadData(){
    bool bResult= EEPROM_readAnything(0,_config);
#ifdef DEBUG
    Serial.println("Loading data");
    Serial.print("Shared key: ");print(_config.keys[0].sharedKey,KEY_SIZE);
    Serial.print("Remote ID: ");print(_config.keys[0].peerId,IDLENGTH);
#endif
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

//For new IDs a new entry for key storage will be created.  If the ID is known, then the key value will be updated.
void Configuration::addKey(const byte *remoteId, const byte* key)
{
    byte index=0;
    //Check if remote ID is already known
    for(byte i=0;i<KEY_COUNT;i++)
    {
        if(_config.keys[i].keyValid && (!memcmp(_config.keys[i].peerId, remoteId, IDLENGTH)))
        {
            index=i;
            break;
        }
    }
    memcpy(_config.keys[index].peerId, remoteId, IDLENGTH);
    memcpy(_config.keys[index].sharedKey, key, KEY_SIZE);
    _config.keys[index].keyValid=true;
    saveData();
}

byte* Configuration::getDefaultKey()
{
    return _config.keys[0].keyValid ? _config.keys[0].sharedKey : 0;
}

byte* Configuration::getDefaultId()
{
    return _config.keys[0].keyValid ? _config.keys[0].peerId : 0;
}

byte Configuration::getIdLength()
{
    return IDLENGTH;
}

byte* Configuration::findKey(const byte* remoteId, byte length)
{
    for(byte i=0;i<KEY_COUNT;i++)
    {
        if(_config.keys[i].keyValid && (!memcmp(_config.keys[i].peerId, remoteId, length)))
        {
            return _config.keys[i].sharedKey;
        }
    }
    return 0;
}

void Configuration::removeAllKeys()
{
    for(byte i=0;i<KEY_COUNT;i++)
    {
        _config.keys[i].keyValid==false;
    }
    saveData();
}


void Configuration::saveData(){
#ifdef DEBUG
    Serial.println("Saving data");
    print(_config.keys[0].sharedKey,16);
#endif
    EEPROM_writeAnything(0,_config);
}


