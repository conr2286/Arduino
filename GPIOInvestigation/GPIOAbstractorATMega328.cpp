/**
  Implementation of the General Purpose I/O (GPIO) abstractor for the ATMega328 MCU

  Provides a uniform interface through class methods, including attachInterupt(),
  similar to the Arduino core's similarly named functions, supporting pin change
  interrupts for all GPIO pins.  Note that the abstractor's interface is defined
  in GPIOAbstractor.h and this source file merely contains an implementation for
  a single MCU.

  Limitations:  Excepting digital pins 2 and 3, this implementation's support for the
  Arduino-defined interrupt modes is limited to RISING, FALLING and CHANGE.  The LOW
  and HIGH modes are not supported; requests for them are silently ignored.

  ATmega328:  This MCU classifies interrupts from GPIO pins as "external" or "pin change".
  The ATmega328 supports only two external interrupts on digital pins 2 and 3, but these
  may be triggered by a LOW, RISING, FALLING or any CHANGE in those pins' logic levels. The
  ATmega328 supports only CHANGE interrupts on the remaining GPIO ports.  The MCU maps
  those pins' bits into three ports, port B, C and D (note:  the ATmega328 lacks port A).  
  Each of these three ports has an interrupt vector in the hardware; they are PCINT0_vect
  (port B), PCINT1_vect (port C), and PCINT2_vect (port C).  When a pin serviced by one of
  these ports changes, the MCU invokes that port’s ISR which must determine which of the 
  8 pins serviced by that port caused the interrupt on that port.

  Implementation:  We implement the ISRs for all three GPIO ports here.  Each interrupt
  handler compares the current state of its port's pins with the saved state of those
  pins from the previous interrupt.  The changed pins are identified and the user's
  supplied interrupt vector(s) invoked as governed by the requested modes.
 
**/


//Definitions for this implementation's target MCU(s)
#ifdef ARDUINO_ARCH_AVR

#include <Arduino.h>
#include "GPIOAbstractor.h"



#ifdef GPIO_DEBUG
volatile unsigned nPortB = 0;
volatile unsigned nPortC = 0;
volatile unsigned nPortD = 0;
#endif


//Implement the class members for servicing the user's pin change interrupt
void (*GPIOAbstractor::interruptHandlers[NPORTS][8])();           //The ATMega328P has 8 digital pins in three PORTs
volatile uint8_t GPIOAbstractor::savedPinStates[NPORTS];          //Saved pin values (for abstracting CHANGE mode)
GPIO_ABSTRACTOR_PINMODE GPIOAbstractor::pinModes[NPORTS][8];      //Saved pin modes supplied by user

//Define the map of digital pin numbers to <port,bit>.  PortB is port#0, PortC is port#1, and PortD is port#2.
//The least significant bit (LSB) in a port is bit 0.  Bit 7 is the most significant bit (MSB).  The map
//is constructed as an array of bit fields where pinMap[0] refers to Digital Pin D0, pinMap[1] to Digital
//Pin D1, etc.  Each entry consists of two fields identifying the port number (PORTB==0, PORTC==1, PORTD==2)
//and the bit within that port controlling a digital pin.  The entire map requires only 24 bytes.  De nada.
const PinMap GPIOAbstractor::pinMap[NPORTS * 8] = {
  { 2, 0 }, //PORTD D0, PCINT16, RX
  { 2, 1 }, //PORTD D1, PCINT17, TX
  { 2, 2 }, //PORTD D2, PCINT18, INT0
  { 2, 3 }, //PORTD D3, PCINT19, INT1, PWM
  { 2, 4 }, //PORTD D4, PCINT20
  { 2, 5 }, //PORTD D5, PCINT21, PWM
  { 2, 6 }, //PORTD D6, PCINT22, PWM
  { 2, 7 }, //PORTD D7, PCINT23
  { 0, 0 }, //PORTB D8, PCINT0
  { 0, 1 }, //PORTB D9, PCINT1, PWM
  { 0, 2 }, //PORTB D10, PCINT2, PWM, SS
  { 0, 3 }, //PORTB D11, PCINT3, PWM, MOSI
  { 0, 4 }, //PORTB D12, PCINT4, MISO
  { 0, 5 }, //PORTB D13, PCINT5, SCK
  { 0, 6 }, //PORTB XTAL1, PCINT6  
  { 0, 7 }, //PORTB XTAL2, PCINT7
  { 1, 0 }, //PORTC A0, PCINT8
  { 1, 1 }, //PORTC A1, PCINT9
  { 1, 2 }, //PORTC A2, PCINT10
  { 1, 3 }, //PORTC A3, PCINT11
  { 1, 4 }, //PORTC A4, PCINT12, SDA
  { 1, 5 }, //PORTC A5, PCINT13, SCL
  { 1, 6 }, //PORTC RESET
  { 1, 7 }  //PORTC N/A?
};



/**
  Constructor initializes the class member variables
**/
GPIOAbstractor::GPIOAbstractor() {

  //Initialize each port
  for (uint8_t portX = 0; portX < NPORTS; portX++) {

    //Save the initial pin conditions from each port so the interrupt handlers can
    //compare their future state with their initial state.
    savedPinStates[0] = PINB;
    savedPinStates[1] = PINC;
    savedPinStates[2] = PIND;

    //Initialize the vectors and modes for each GPIO pin serviced by this port
    for (uint8_t bit = 0; bit < 8; bit++) {
      interruptHandlers[portX][bit] = NULL;   //User has not yet supplied an interrupt vector for this pin
      pinModes[portX][bit] = 0;               //User has not yet supplied a mode for this pin
    }

  }  //portX

}  //GPIOAbstractor()



/**
  Configures the specified pin for OUTPUT, INPUT or INPUT_PULLUP
**/
void GPIOAbstractor::pinMode(uint8_t pin, GPIO_ABSTRACTOR_PINMODE mode) {
  ::pinMode(pin, mode);  //We don't add value to this method for the 328
}


/**
  Reads the value of the specified digital pin
**/
GPIO_ABSTRACTOR_PINSTATUS GPIOAbstractor::digitalRead(uint8_t pin) {
  return ::digitalRead(pin);  //We don't add value to this method for the 328
}


/**
  Reads the value of the specified digital pin
**/
void GPIOAbstractor::digitalWrite(uint8_t pin, GPIO_ABSTRACTOR_PINSTATUS value) {
  ::digitalWrite(pin, value);  //We don't add value to this method for the 328
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
  //for (int i = 0; i < 24; i++) {
  //  Serial.print(i); Serial.print(" = <"); Serial.print(pinMap[i].port); Serial.print(","); Serial.print(pinMap[i].bit); Serial.print(">\n");
  //}


  //Let the Arduino core process requests for the so-called "external" interrupts as our overhead doesn't add value
  if ((digitalPin == 2) || (digitalPin == 3)) {
    ::attachInterrupt(digitalPinToInterrupt(digitalPin), isr, intMode);
  } else {

    //Ignore requests for illegal digital pin numbers
    if (digitalPin > 23) return;

    //Map digitalPin number to port and bit within that port
    uint8_t port = pinMap[digitalPin].port;  //Example:  PORTB (PCMSK1) handles digital pin 9
    uint8_t bit = pinMap[digitalPin].bit;    //Example:  Bit 1 handles digital pin 9

    //Record user-requested mode and user's ISR for this pin
    GPIOAbstractor::pinModes[port][bit] = intMode;
    GPIOAbstractor::interruptHandlers[port][bit] = isr;

    Serial.print("port=");
    Serial.print(port), Serial.print(" bit=");
    Serial.print(bit);
    Serial.print("\n");

    //If port was previously enabled, then this pin in that port can race during setup below
    noInterrupts();

    //Enable this port's interrupt vector
    PCICR |= (1 << port);  

    //Enable pin change interrupts.  PCINT0..PCINT23 are handled by three vectors, PCINT0..2, each
    //of which can be enabled.  Each vector has a pin change mask controlling which of the 8 possible
    //pins serviced by that vector are actually enabled.
    switch (port) {

      //PORTB services pins 0..7
      case 0:
        PCMSK0 |= (1<<bit);
        savedPinStates[port] = PINB;  //Save state of pins in this port
        break;

      //PORTC services pins 8..14
      case 1:
        PCMSK1 |= (1<<bit);
        savedPinStates[port] = PINC;  //Save state of pins in this port
        break;

      //PORTD services pins 16..23
      case 2:
        PCMSK2 |= (1<<bit);
        savedPinStates[port] = PIND;  //Save state of pins in this port
        break;
    }
 
    interrupts();
    Serial.print("PCICR="); Serial.print(PCICR); Serial.print("\n");
    Serial.print("PCMSK2="); Serial.print(PCMSK2); Serial.print("\n");

  }  //pin change interrupt
}





/**
  Pin Change Interrupt handler for PORT B
**/
ISR(PCINT0_vect) {

#ifdef GPIO_DEBUG
  nPortB++;
#endif

  const uint8_t port = 2;  //Servicing interrupts for PORTD

  //Determine which pins' values changed
  uint8_t bitValues = PINB;                                                //Read this port's digital pin bit values
  uint8_t changedPins = bitValues ^ GPIOAbstractor::savedPinStates[port];  //1s identify the changed pins
  GPIOAbstractor::savedPinStates[port] = bitValues;                        //Save their current state for the next interrupt

  //Consider action for each pin serviced by this PORT (bit 0 is LSB)
  for (uint8_t bit = 0; bit < 8; bit++) {

    //Has the user supplied an interrupt handler for this bit of this port?
    if (GPIOAbstractor::interruptHandlers[port][bit] != NULL) {

      //Yes, we have an interrupt handler for this pin.  Did this pin's value change?
      if (changedPins & (1 << bit)) {

        uint8_t mode = GPIOAbstractor::pinModes[port][bit];  //When does user want notified?
        uint8_t pinValue = bitValues & (1 << bit);         //The value of this digital pin

        //  mode        event
        //  CHANGE      pinValue changed      ==>    invoke user's handler
        //  RISING      pinValue changed to 1 ==>    invoke user's handler
        //  FALLING     pinValue changed to 0 ==>    invoke user's handler
        if ((mode == CHANGE) || ((mode == RISING) && (pinValue != 0)) || ((mode == FALLING) && (pinValue == 0))) {
          GPIOAbstractor::interruptHandlers[port][bit]();  //Invoke user-supplied ISR
        }

      }  //pin value changed

    }  //user-supplied interrrupt handler

  }  //bit


}  //PCINT0_vect








/**
  Pin Change Interrupt handler for PORT C
**/
ISR(PCINT1_vect) {

#ifdef GPIO_DEBUG
  nPortC++;
#endif

  const uint8_t port = 2;  //Servicing interrupts for PORTD

  //Determine which pins' values changed
  uint8_t bitValues = PINC;                                                //Read this port's digital pin bit values
  uint8_t changedPins = bitValues ^ GPIOAbstractor::savedPinStates[port];  //1s identify the changed pins
  GPIOAbstractor::savedPinStates[port] = bitValues;                        //Save their current state for the next interrupt

  //Consider action for each pin serviced by this PORT (bit 0 is LSB)
  for (uint8_t bit = 0; bit < 8; bit++) {

    //Has the user supplied an interrupt handler for this bit of this port?
    if (GPIOAbstractor::interruptHandlers[port][bit] != NULL) {

      //Yes, we have an interrupt handler for this pin.  Did this pin's value change?
      if (changedPins & (1 << bit)) {

        uint8_t mode = GPIOAbstractor::pinModes[port][bit];  //When does user want notified?
        uint8_t pinValue = bitValues & (1 << bit);         //The value of this digital pin

        //  mode        event
        //  CHANGE      pinValue changed      ==>    invoke user's handler
        //  RISING      pinValue changed to 1 ==>    invoke user's handler
        //  FALLING     pinValue changed to 0 ==>    invoke user's handler
        if ((mode == CHANGE) || ((mode == RISING) && (pinValue != 0)) || ((mode == FALLING) && (pinValue == 0))) {
          GPIOAbstractor::interruptHandlers[port][bit]();  //Invoke user-supplied ISR
        }

      }  //pin value changed

    }  //user-supplied interrrupt handler

  }  //bit


}  //PCINT1_vect






/**
  Pin Change Interrupt handler for PORT D)
**/
ISR(PCINT2_vect) {

#ifdef GPIO_DEBUG
  nPortD++;
#endif

  const uint8_t port = 2;  //Servicing interrupts for PORTD

  //Determine which pins' values changed
  uint8_t bitValues = PIND;                                                //Read this port's digital pin bit values
  uint8_t changedPins = bitValues ^ GPIOAbstractor::savedPinStates[port];  //1s identify the changed pins
  GPIOAbstractor::savedPinStates[port] = bitValues;                        //Save their current state for the next interrupt

  //Consider action for each pin serviced by this PORT (bit 0 is LSB)
  for (uint8_t bit = 0; bit < 8; bit++) {

    //Has the user supplied an interrupt handler for this bit of this port?
    if (GPIOAbstractor::interruptHandlers[port][bit] != NULL) {

      //Yes, we have an interrupt handler for this pin.  Did this pin's value change?
      if (changedPins & (1 << bit)) {

        uint8_t mode = GPIOAbstractor::pinModes[port][bit];  //When does user want notified?
        uint8_t pinValue = bitValues & (1 << bit);         //The value of this digital pin

        //  mode        event
        //  CHANGE      pinValue changed      ==>    invoke user's handler
        //  RISING      pinValue changed to 1 ==>    invoke user's handler
        //  FALLING     pinValue changed to 0 ==>    invoke user's handler
        if ((mode == CHANGE) || ((mode == RISING) && (pinValue != 0)) || ((mode == FALLING) && (pinValue == 0))) {
          GPIOAbstractor::interruptHandlers[port][bit]();  //Invoke user-supplied ISR
        }

      }  //pin value changed

    }  //user-supplied interrrupt handler

  }  //bit

}  //PCINT2_vect



#endif  //ARDUINO_ARCH_AVR
