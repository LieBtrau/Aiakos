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
#ifndef ARDUINO_SAM_DUE
    bool setRfidKey(byte key[]);
    bool equalsRfidKey(byte key[]);
    word getRfidHandle();
    word getIasHandle();
    void setRfidHandle(word handle);
    void setIasHandle(word handle);
    static const byte RFID_KEY_SIZE=5;
#endif
private:
    static const byte KEY_SIZE=16;
    static const byte KEY_COUNT=2;
    typedef struct
    {
        byte sharedKey[KEY_SIZE];
        byte peerId[IDLENGTH];
    }SHARED_KEY;
#ifndef ARDUINO_SAM_DUE
    typedef struct
    {
        byte rfidkey[RFID_KEY_SIZE];
        bool keyValid;
    }RFID;
#endif
    typedef struct
    {
        byte nrOfValidKeys;
        SHARED_KEY keys[KEY_COUNT];
#ifndef ARDUINO_SAM_DUE
        word handleRfid;
        word handleIas;
        RFID rfid;
#endif
    }CONFIG;
    void initializeEEPROM();
    void saveData();
    CONFIG _config;
 };

#endif // CONFIGURATION_H

