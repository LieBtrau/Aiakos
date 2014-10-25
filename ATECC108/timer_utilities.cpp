#include "timer_utilities.h"
#include "Arduino.h"

void delay_10us(uint8_t delay){
    delayMicroseconds(10*delay );
}

void delay_ms(uint8_t delayms){
    delay(delayms);
}

