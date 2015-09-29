//declaration of needed libraries must be done in the ino-file of the project.

#include <Wire.h>
#include <SPI.h>
#include <EEPROM.h>

//git clone git@github.com:LieBtrau/arduino-xterm.git ~/git/arduino-xterm
//ln -s ~/git/arduino-xterm/Xterm ~/Arduino/libraries
#include <Xterm.h>
//git clone git@github.com:LieBtrau/PN532.git ~/git/PN532
//ln -s ~/git/PN532/PN532 ~/Arduino/libraries/
//ln -s ~/git/PN532/PN532_SPI/ ~/Arduino/libraries/
//ln -s ~/git/PN532/NDEF/ ~/Arduino/libraries/
#include <PN532_SPI.h>
#include <PN532.h>
#include <NfcAdapter.h>
//git clone git@github.com:LieBtrau/RadioHead.git ~/git/RadioHead
//ln -s ~/git/RadioHead/ ~/Arduino/libraries/
#include "RadioHead.h"
//git clone git@github.com:wastel7/microBox.git ~/git/microBox
//ln -s ~/git/microBox/ ~/Arduino/libraries/
#include "microBox.h"
//git clone git@github.com:LieBtrau/Arduino_STM32.git ~/git/Arduino_STM32
//ln -s ~/git/Arduino_STM32/ ~/Programs/arduino-1.6.5/hardware/

//Build instruction:
//~/Programs/arduino-1.6.5/arduino --verify --board Arduino_STM32:STM32F1:nucleo_f103rb --pref target_package=Arduino_STM32 --pref build.path=./build --pref target_platform=STM32F1 --pref board=nucleo_f103rb ~/Arduino/blinky_nucleo/blinky_nucleo.ino
