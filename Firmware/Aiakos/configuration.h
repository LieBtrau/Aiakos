#ifndef CONFIGURATION_H
#define CONFIGURATION_H
class Configuration
{
public:
    static const byte IDLENGTH=9;
    Configuration();
    bool loadData();
    bool init();
    static byte getIdLength();
    void addKey(const byte* remoteId, const byte* key);
    void removeAllKeys();
    byte* getDefaultKey();
    byte* getDefaultId();
    byte* findKey(const byte* remoteId, byte length);
    bool setRfidKey(byte key[]);
    bool getRfidKey(byte key[]);
private:
    static const byte KEY_SIZE=16;
    static const byte KEY_COUNT=2;
    typedef struct
    {
        byte sharedKey[KEY_SIZE];
        byte peerId[IDLENGTH];
    }SHARED_KEY;
    typedef struct
    {
        byte nrOfValidKeys;
        SHARED_KEY keys[KEY_COUNT];
        byte rfidkey[4];
    }CONFIG;
    void initializeEEPROM();
    void saveData();
    CONFIG _config;
 };

#endif // CONFIGURATION_H

