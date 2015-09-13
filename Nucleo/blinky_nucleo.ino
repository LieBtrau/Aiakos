//declaration of needed libraries must be done in the ino-file of the project.
#include <Wire.h>
#include <SPI.h>
//git clone git@github.com:LieBtrau/PN532.git ~/Arduino/libraries
#include <PN532_SPI.h>
#include <PN532.h>
#include <NfcAdapter.h>
//git clone git@github.com:LieBtrau/RadioHead.git ~/Arduino/libraries
#include "RadioHead.h"

//Build instruction:
//Download Arduino STM32 from Github and copy it into the Arduino hardware folder
//Build instruction: ~/Programs/arduino-1.6.5/arduino --verify --board Arduino_STM32:STM32F1:nucleo_f103rb --pref target_package=Arduino_STM32 --pref build.path=./build --pref target_platform=STM32F1 --pref board=nucleo_f103rb ~/Arduino/blinky_nucleo/blinky_nucleo.ino
