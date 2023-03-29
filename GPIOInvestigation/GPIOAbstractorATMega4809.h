/**

  @file Device-Dependent Declarations for the ATMega4809 implementation of the GPIO Abstractor

  Implementation:  These declarations are #included by the device-independent GPIOAbstractor.h file.
  The ATMega4809 offers several GPIO enhancements over the venerable ATMega328:
    + 41 GPIO pins on the chipset though the Arduino Nano Every (and this implementation) exposes only D0..D21
    + Each pin supports Arduino interrupt modes, RISING, FALLING, CHANGE, and LOW
    + Each port has an associated INTFLAGS register indicating which pin triggered the interrupt
  The Arduino core supports interrupts on all digital pins for the Nano Every, so we pass the "real" 
  work to the core for implementation.  Our value added is merely the uniform interface declared by 
  the GPIOAbstractor that supports several MCUs.

**/

#pragma once

//This Arduino implementation uses enumerations for PinMode and PinStatus; other MCUs use uint8_t.
//Yes, the data *types* for these API parameters are MCU dependent.  Really.  So we work around
//the differences by #defining symbols to the respective MCU datatypes in each MCU's device-specific
//declaration file.
#define GPIO_ABSTRACTOR_PINMODE   PinMode
#define GPIO_ABSTRACTOR_PINSTATUS PinStatus


class GPIOAbstractor {

  //This private section contains device-dependent declarations for this specific MCU
private:
 

  //This public section contains device-independent declarations comprising the interface to the GPIOAbstractor.
  //In the best of all possible worlds, these would reside in the device-independent GPIOAbstractor.h but I've
  //never found a clean way to do that with C++.  So... please avoid placing device-dependent declarations in
  //the class's public interface.
public:
  GPIOAbstractor();
  static void pinMode(uint8_t pin, GPIO_ABSTRACTOR_PINMODE mode);
  static GPIO_ABSTRACTOR_PINSTATUS digitalRead(uint8_t pin);
  static void digitalWrite(uint8_t pin, GPIO_ABSTRACTOR_PINSTATUS value);
  static void attachInterrupt(uint8_t digitalPin, void (*isr)(), GPIO_ABSTRACTOR_PINSTATUS intMode);
  static void detachInterrupt(uint8_t digitalPin);
};
