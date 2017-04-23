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
private:
    static const byte KEY_SIZE=16;
    static const byte KEY_COUNT=2;
    typedef struct
    {
        byte sharedKey[KEY_SIZE];
        byte peerId[IDLENGTH];
        bool keyValid;
    }SHARED_KEY;
    typedef struct
    {
        SHARED_KEY keys[KEY_COUNT];
    }CONFIG;
    void initializeEEPROM();
    CONFIG _config;
    void saveData();
};

#endif // CONFIGURATION_H

