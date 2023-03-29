/**
  Implementation of the General Purpose I/O (GPIO) abstractor for the ATMega4809 MCU

  Provides a uniform interface through class methods, including attachInterupt(),
  similar to the Arduino core's similarly named functions, supporting pin change
  interrupts for all GPIO pins.  Note that the abstractor's interface is defined
  in GPIOAbstractor.h and this source file merely contains an implementation for
  a single MCU.

  ATmega4809:  This MCU supports LOW, RISING, FALLING and CHANGE interrupt modes on
  all GPIO pins.  The MCU maps those pins' bits into six ports, port A, B, C, D, E and F.
  Each of these ports has an interrupt vector in the hardware.  When a pin serviced by
  any port changes, the MCU invokes that port’s ISR which determines which of the 
  8 pins serviced by that port caused the interrupt on that by examining that port's
  INTFLAGS register.

  Implementation:  Since the Arduino core provides satisfactory support for the ATMega4809
  (e.g. the Arduino Nano Every), we merely pass through the "real" work to the core.  All
  we're doing is implementing the interface defined by the GPIOAbstractor.
 
**/


//This implementation targets the MEGA AVR boards, specifically the Arduino Nano Every 
#ifdef ARDUINO_ARCH_MEGAAVR

#include <Arduino.h>
#include "GPIOAbstractor.h"



/**
  Constructor initializes the class member variables
**/
GPIOAbstractor::GPIOAbstractor() {

  //Nothing to do, eh?

}  //GPIOAbstractor()



/**
  Configures the specified pin for OUTPUT, INPUT or INPUT_PULLUP
**/
void GPIOAbstractor::pinMode(uint8_t pin, GPIO_ABSTRACTOR_PINMODE mode) {
  ::pinMode(pin, mode);  //We don't add value to this method
}


/**
  Reads the value of the specified digital pin
**/
GPIO_ABSTRACTOR_PINSTATUS GPIOAbstractor::digitalRead(uint8_t pin) {
  return ::digitalRead(pin);  //We don't add value to this method
}


/**
  Reads the value of the specified digital pin
**/
void GPIOAbstractor::digitalWrite(uint8_t pin, GPIO_ABSTRACTOR_PINSTATUS value) {
  ::digitalWrite(pin, value);  //We don't add value to this method
}





/**
  Attach an interrupt handler to the specified digital pin

  This class method provides a uniform interface to attach an interrupt handler to
  a digital input pin, including the so-called pin-change interrupts.  Errors are
  silently ignored.

  @param digitalPin The GPIO digital pin (not interrupt) numer
  @param isr User-supplied ISR to handle interrupts on digitalPin
  @param intMode Supported modes are RISING, FALLING, CHANGE

**/
void GPIOAbstractor::attachInterrupt(uint8_t digitalPin, void (*isr)(), GPIO_ABSTRACTOR_PINSTATUS intMode) {

  Serial.print("attachInterrupt ");
  Serial.print(digitalPin);
  Serial.print(" ");
  Serial.print(intMode);
  Serial.print("\n");

  //Let the Arduino core process this request since it supports interrupts on all ATMega digital pins
  ::attachInterrupt(digitalPinToInterrupt(digitalPin), isr, intMode);

}  //attachInterrupt()

#endif  //ARDUINO_ARCH_MEGAAVR
