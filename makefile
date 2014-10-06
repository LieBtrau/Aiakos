
ARDUINO_DIR  = /home/christoph/Programs/arduino-1.5.2

TARGET       = Aiakos
ARDUINO_LIBS =  hardware/arduino/avr/libraries/SoftwareSerial hardware/arduino/avr/libraries/SPI
LOCAL_LIBS = ./RadioHead
LOCAL_STATIC_LIBS =

BOARD_TAG    = uno
ARDUINO_PORT = /dev/ttyACM0

include Arduino.mk


