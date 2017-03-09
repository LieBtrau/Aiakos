#ifndef CONFIGURATION_H
#define CONFIGURATION_H
class Configuration
{
public:
    static const byte IDLENGTH=9;
    Configuration();
    void saveData();
    bool loadData();
    bool init();
    void setKey(byte index, const byte* id, const byte* key);
    byte* getKey(byte index);
    byte* getId(byte index);
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
};

#endif // CONFIGURATION_H

