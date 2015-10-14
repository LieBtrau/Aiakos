//declaration of needed libraries must be done in the ino-file of the project.

#include <Wire.h>
#include <SPI.h>
#include <EEPROM.h>

//git clone git@github.com:LieBtrau/PN532.git ~/git/PN532
//ln -s ~/git/PN532/PN532 ~/Arduino/libraries/
//ln -s ~/git/PN532/PN532_SPI/ ~/Arduino/libraries/
#include <PN532_SPI.h>
#include <PN532.h>

//git clone git@github.com:LieBtrau/NDEF.git ~/git/NDEF
//ln -s ~/git/NDEF/ ~/Arduino/libraries/
#include <NfcAdapter.h>

//git clone git@github.com:LieBtrau/RadioHead.git ~/git/RadioHead
//ln -s ~/git/RadioHead/ ~/Arduino/libraries/
#include "RadioHead.h"

//git clone git@github.com:LieBtrau/microBox.git ~/git/microBox
//ln -s ~/git/microBox/ ~/Arduino/libraries/
#include "microBox.h"

//~/git$ git clone git@github.com:LieBtrau/micro-ecc.git ~/git/uECC
//ln -s ~/git/uECC/ ~/Arduino/libraries/
#include "uECC.h"

//git clone git@github.com:adamvr/arduino-base64.git ~git/arduino-base64
//ln -s ~/git/arduino-base64 ~/Arduino/libraries/
#include "Base64.h"

//git clone git@github.com:LieBtrau/Arduino_STM32.git ~/git/Arduino_STM32
//ln -s ~/git/Arduino_STM32/ ~/Programs/arduino-1.6.5/hardware/

//Build instruction:
//~/Programs/arduino-1.6.5/arduino --verify --board Arduino_STM32:STM32F1:nucleo_f103rb --pref target_package=Arduino_STM32 --pref build.path=./build --pref target_platform=STM32F1 --pref board=nucleo_f103rb ~/Arduino/blinky_nucleo/blinky_nucleo.ino
