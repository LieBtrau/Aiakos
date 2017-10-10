#include "EEPROMAnything.h"
#include "configuration.h"
#include "debug.h"

Configuration::Configuration()
{
    _config.nrOfValidKeys=0;
}

void Configuration::initializeEEPROM(){
#ifdef ARDUINO_STM_NUCLEO_F103RB || defined(ARDUINO_GENERIC_STM32F103C)
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
        debug_println("Loading: config invalid");
    }
    debug_println("Loading data");
    debug_print("Number of valid keys: "); debug_println(_config.nrOfValidKeys, DEC);
    if(_config.nrOfValidKeys)
    {
        debug_print("Shared key: ");print(_config.keys[0].sharedKey,KEY_SIZE);
        debug_print("Remote ID: ");print(_config.keys[0].peerId,IDLENGTH);
    }
#ifndef ARDUINO_SAM_DUE
    debug_print("rfidkey");print(_config.rfid.rfidkey, RFID_KEY_SIZE);
    debug_print("rfidhandle: ");debug_println(_config.handleRfid, DEC);
    debug_print("iashandle: ");debug_println(_config.handleIas, DEC);

#endif
#endif
    return bResult;
}

bool Configuration::init(){
    //Set up for STM32F103RB (see Arduino STM32 example code)
#ifdef ARDUINO_STM_NUCLEO_F103RB || defined(ARDUINO_GENERIC_STM32F103C)
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
    return (_config.nrOfValidKeys ? _config.keys[0].sharedKey : 0);
}

byte* Configuration::getDefaultId()
{
    return (_config.nrOfValidKeys ? _config.keys[0].peerId : 0);
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
#ifndef ARDUINO_SAM_DUE
    debug_println(_config.handleRfid, DEC);
    debug_println(_config.handleIas, DEC);
#endif
    EEPROM_writeAnything(0,_config);
}

#ifndef ARDUINO_SAM_DUE
bool Configuration::setRfidKey(byte key[])
{
    memcpy(_config.rfid.rfidkey, key, RFID_KEY_SIZE);
    _config.rfid.keyValid=true;
    saveData();
    return true;
}

bool Configuration::equalsRfidKey(byte key[])
{
    if(!_config.rfid.keyValid)
    {
        return false;
    }
    if(memcmp(_config.rfid.rfidkey, key, RFID_KEY_SIZE))
    {
        debug_println("Wrong data doesn't equal key");
        debug_printArray(key, RFID_KEY_SIZE);
        return false;
    }
    return true;
}

word  Configuration::getRfidHandle()
{
    return _config.handleRfid;
}

word  Configuration::getIasHandle()
{
    return _config.handleIas;
}

void  Configuration::setRfidHandle(word handle)
{
    _config.handleRfid=handle;
    saveData();
}

void  Configuration::setIasHandle(word handle)
{
    _config .handleIas=handle;
    saveData();
}


#endif


