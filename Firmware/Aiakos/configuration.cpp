#include "EEPROMAnything.h"
#include "configuration.h"
#include "debug.h"

Configuration::Configuration()
{
    _config.nrOfValidKeys=0;
}

void Configuration::initializeEEPROM(){
#ifdef ARDUINO_STM_NUCLEO_F103RB
    EEPROM.format();
#endif
    saveData();
}

bool Configuration::loadData(){
    CONFIG testcfg;
    bool bResult= EEPROM_readAnything(0,testcfg) && testcfg.nrOfValidKeys<=KEY_COUNT;
    if(bResult)
    {
        //Make sure only valid data is used in the application
        _config=testcfg;
    }
#ifdef DEBUG
    else
    {
        Serial.println("Loading: config invalid");
    }
    Serial.println("Loading data");
    Serial.print("Number of valid keys: "); print(&_config.nrOfValidKeys, 1);
    if(_config.nrOfValidKeys)
    {
        Serial.print("Shared key: ");print(_config.keys[0].sharedKey,KEY_SIZE);
        Serial.print("Remote ID: ");print(_config.keys[0].peerId,IDLENGTH);
    }
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
        debug_println("EEPROM data corrupt!");
        initializeEEPROM();
        return false;
    }
    return true;
}

//For new IDs a new entry for key storage will be created.  If the ID is known, then the key value will be updated.
void Configuration::addKey(const byte *remoteId, const byte* key)
{
    byte index=0;
    bool bKeyFound=false;

    //Check if remote ID is already known
    for(byte index=0;index<_config.nrOfValidKeys;index++)
    {
        if(!memcmp(_config.keys[index].peerId, remoteId, IDLENGTH))
        {
            bKeyFound=true;
            break;
        }
    }
    if(!bKeyFound)
    {
        index=_config.nrOfValidKeys++;
    }
    memcpy(_config.keys[index].peerId, remoteId, IDLENGTH);
    memcpy(_config.keys[index].sharedKey, key, KEY_SIZE);
    debug_print("Storing key at location: ");
    debug_println(index, DEC);
    saveData();
}

byte* Configuration::getDefaultKey()
{
    return _config.nrOfValidKeys ? _config.keys[0].sharedKey : 0;
    }

    byte* Configuration::getDefaultId()
    {
    return _config.nrOfValidKeys ? _config.keys[0].peerId : 0;
}

byte Configuration::getIdLength()
{
    return IDLENGTH;
}

byte* Configuration::findKey(const byte* remoteId, byte length)
{
    for(byte i=0;i<_config.nrOfValidKeys;i++)
    {
        if(!memcmp(_config.keys[i].peerId, remoteId, length))
        {
            return _config.keys[i].sharedKey;
        }
    }
    return 0;
}

void Configuration::removeAllKeys()
{
    _config.nrOfValidKeys=0;
    saveData();
}


void Configuration::saveData(){
    debug_println("Saving data");
    debug_printArray(&_config.nrOfValidKeys,1);
    debug_printArray(_config.keys[0].sharedKey,16);
    EEPROM_writeAnything(0,_config);
}


