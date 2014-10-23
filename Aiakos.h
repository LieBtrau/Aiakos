/*  Garage door opener + remote control application
    Copyright (C) 2014  Christoph Tack

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef Aiakos_H_
#define Aiakos_H_

//add your includes for the project here
#include "Arduino.h"
#include <stddef.h>            // data type definitions
#include "RDM630/rdm630.h"
#include "ATECC108/ecc108_physical.h"   // function definitions for the physical layer

//end of add your includes here
#ifdef __cplusplus
extern "C" {    //define avr-gcc compiled headers & sources here
#endif

#include "ATECC108/ecc108_examples.h"  // definitions and declarations for example functions

void loop();
void setup();
#ifdef __cplusplus
} // extern "C"
#endif

//add your function definitions for the project here




//Do not add code below this line
#endif /* Aiakos_H_ */

