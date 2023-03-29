/**
  @file General Purpose I/O (GPIO) abstractor with support for Pin Change Interrupts

  Attempts to provide a uniform interface, including pin change interrupts,
  to the various MCU chips employed in the Arduino controllers
 
  Implementation:  We are using a class merely to organize our namespace with
  class members --- there's little other advantage over C11 here.  In the
  best of all possible worlds, we would place the MCU-independent declarations
  here and the MCU-dependent declarations separately.  If you figure out a
  clean way to do this in C++, perhaps you'll reorganize it that way; until
  then, all we can really do is abstract the MCU dependencies, and avoid
  polluting the public class interface with device-dependent members.

**/ 

#pragma once

//Debugging controls and variables
//#define GPIO_DEBUG 

//Include the declarations for the ATMega328 MCU implementation
#ifdef ARDUINO_ARCH_AVR
#include "GPIOAbstractorATMega328.h"
#endif

//Include the declarations for the ATMega4809 MCU implementation
#ifdef ARDUINO_ARCH_MEGAAVR
#include "GPIOAbstractorATMega4809.h"
#endif


//Add the #includes for additional MCUs' declarations below...
