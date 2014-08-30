
ARDUINO_DIR  = /home/christoph/Programs/arduino-1.5.2

TARGET       = Aiakos
ARDUINO_LIBS =  hardware/arduino/avr/libraries/SoftwareSerial

BOARD_TAG    = uno
ARDUINO_PORT = /dev/ttyACM0


include $(ARDUINO_DIR)/Arduino.mk


