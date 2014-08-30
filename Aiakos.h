// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef Aiakos_H_
#define Aiakos_H_

#include "Arduino.h"

//add your includes for the project here


//end of add your includes here
#ifdef __cplusplus
extern "C" {
#endif

typedef enum{
    WAITING_FOR_STX,
    READING_DATA,
    DATA_VALID
}state;

void loop();
void setup();
#ifdef __cplusplus
} // extern "C"
#endif

//add your function definitions for the project here




//Do not add code below this line
#endif /* Aiakos_H_ */

