/**
   @file Optionally interrupt-driven Shaft Encoder implementation over the Arduino APIs
   
   @author  Jim Conrad, KQ7B

   Dependencies: This code was originally developed for the Yumo E6A2-CW3C (Sparkfun Part# COM-10932) 
   200 pulse/revolution quadrature shaft encoder, but likely will work with many
   quadrature encoders.  Designed for moderately slow rotation speeds, this code eschews
   direct access to GPIO pins to favor the modest processor independence provided by the 
   Arduino APIs.
   
   Operation:  Quadrature shaft encoders output two signals, referred to here as pinA and
   pinB.  The E6A2-CW3C signals are NPN open collector, and we arrange for the chip to supply
   the necessary pullup resistors on the two GPIO pins.  We decode the two-channel quadrature
   signal with the usual finite state machine (FSM).  When requested, the interrupt system
   drives the FSM, consuming two interrupt-capable digital pins (e.g. 2 and 3), expecting
   to track the encoder's position more accurately than polling alone. 

   Oddities:  The extensive use of static data/methods arises because the interrupt service 
   routine doesn't have a reference to "this" object.
**/

//This implementation of the ShaftEncoder is for AVR architectures
#ifdef ARDUINO_ARCH_AVR

#include <Arduino.h>
#include <ShaftEncoder.h>


//Many variables that seemingly should be member variables are static because the ISR must be static
//and thus has no reference to locate this particular instance of a ShaftEncoder
static long encoderPosition;  //Compiler must expect position to change as code executes
static uint8_t currentState;  //Finite State Machine's current state
static unsigned nInvalids;
static uint8_t pinA;  //Encoder's quadrature signal, A, is on pinA
static uint8_t pinB;  //Encoder's quadrature signal, B, is on pinB

//Define the possible actions driven by the finite state machine decoding the quadrature signals
#define T_CW 0  //Clockwise rotation
#define T_CC 1  //Counter-clockwise rotation
#define T_BN 2  //Bounce
#define T_IV 3  //Invalid

//Define the Finite State Transition Matrix for decoding the quadrature output from the encoder.  The table
//has four rows, one for each of the machine's four possible current states, 00, 01, 10 and 11.  Each row
//has four columns, one for each event, 00, 01, 10, and 11. An event is a two-bit tuple formed from
//the encoder's quadature outputs read from digital pins, pinA and pinB.  The content of each table cell
//specifies the action to take in response to events in the current state.  The next-state is always the
//value of the event (e.g. 00, 01, 10 or 11) itself.  An event may trigger one of four possible
//actions:  clockwise (T_CW), counter-clockwise (T_CC), bounce (T_BN), invalid (T_IV).  A "bounce" arises
//when the encoder's position is sampled faster than it rotates; we ignore bounces.  An "invalid"
//event arises when the encoder's shaft is rotated faster than sampled --- it's an illegal grey code
//transition in which more than one bit changed, indicating we missed one or more transitions.
static uint8_t stm[4][4] = {
  { T_BN, T_CC, T_CW, T_IV },  //Actions for currentState==00 for events 00, 01, 10 and 11
  { T_CW, T_BN, T_IV, T_CC },  //Actions for currentState==01 for events 00, 01, 10 and 11
  { T_CC, T_IV, T_BN, T_CW },  //Actions for currentState==10 for events 00, 01, 10 and 11
  { T_IV, T_CW, T_CC, T_BN }   //Actions for currentState==11 for events 00, 01, 10 and 11
};



/**
  Read the encoder's quadrature signals and drive the finite state machine
**/
static void pollEncoderPins() {

  //Read value of both encoder signals.  To avoid potential invalid event values, consider changing
  //this code (for rapidly rotating encoders) to read directly from a PORT to get the value of both
  //pins in a single, atomic operation.
  uint8_t rotA = digitalRead(pinA);      //Will be 0 or 1
  uint8_t rotB = digitalRead(pinB);      //Also 0 or 1... BUT... pinA *might* have changed :(
  uint8_t event = (rotA << 1) | (rotB);  //Event value will be 00, 01, 10 or 11

  //Using this event, drive the state machine's actions coded in the state transition matrix, stm[][]
  switch (stm[currentState][event]) {
    case T_CW: encoderPosition++; break;  //Clockwise rotation
    case T_CC: encoderPosition--; break;  //Counter-clockwise rotation
    case T_BN: break;                     //Ignore contact bounce (sampling too fast)
    case T_IV:                            //Invalid transition (we missed a transition)
    default: nInvalids++;                 //Count invalids and ignore unknown rotation
  }

  //Whether valid or not, this event now becomes the state machine's new current state
  currentState = event;                   //"No matter where you go, there you are." -- Buckaroo Bonzai
}


/**
  The encoder's interrupt service routine (ISR)
  
  The ISR is invoked by both the rising and falling edges of both quadrature signals (pinA and
  pinB).  The use of interrupts, when available, helps avoid invalid transitions by allowing
  the rotation of the encoder, rather than program polling alone, drive the FSM.  Sadly, some
  chips support very few external interrupts.
**/
static void encoderISR() {

  //Drive the finite state machine with this interrupt
  pollEncoderPins();

}


/**
   Initialize the ShaftEncoder module 

   Quadrature encoders output two digital signals known here as pin_A and pin_B.  During clockwise
   rotation, transitions in the signal on pin_A leads those on pin_B.  Transitions can be detected
   either by polling (rapidly invoking getPosition()) or by enabling interrupts.  Note that some
   Arduinos have severe limitations on which pins can trigger external interrupts.  We initialize
   the encoder's current position to 0 but you can change it with setPosition().
   @param pin_A Arduino digital pin for the leading encoder signal during clockwise rotation
   @param pin_B The encoder's other signal
   @param interruptEnable true requests interrupts on both pin_A and pin_B
**/
ShaftEncoder::ShaftEncoder(uint8_t pin_A, uint8_t pin_B, bool interruptEnable) {

  //Initialize ShaftEncoder's state
  nInvalids = 0;         //No noise yet
  pinA = pin_A;          //Arduino Digital Pin numbers...
  pinB = pin_B;          //...for the quadrature signals.
  encoderPosition = 0L;  //The encoder's initial position

  //Configure the encoder digital input pins
  pinMode(pinA, INPUT_PULLUP);  //Arrange for Arduino to supply an open collector pullup resistor
  pinMode(pinB, INPUT_PULLUP);  //on both GPIO pins

  //Initialize the FSM's currentState with the encoder's current quadrature signals
  uint8_t rotA = digitalRead(pinA);     //Will be 0 or 1
  uint8_t rotB = digitalRead(pinB);     //Also 0 or 1
  currentState = (rotA << 1) | (rotB);  //Event value will be 00, 01, 10 or 11

  //If the user wants interrupts enabled, then configure interrupt system to detect
  //encoder shaft rotatation by noting the rising/falling edges of the encoder's two signals
  //on pinA and pinB.  Without interrupts, the user must poll rapidly with getPosition() to
  //avoid missed transitions (detected as T_IV events).  The state machine works both ways
  //but typically is more accurate when interrupt rather than polling driven.
  if (interruptEnable) {
    attachInterrupt(digitalPinToInterrupt(pinA), encoderISR, CHANGE);  //Note that a single ISR services both pins
    attachInterrupt(digitalPinToInterrupt(pinB), encoderISR, CHANGE);
  }
}



/**
  Retrieve the shaft encoder's current position
**/
long ShaftEncoder::getPosition() {
  pollEncoderPins();
  return encoderPosition;
}


/**
  Set the shaft encoder's current position

  @param p the value to be recorded
**/
void ShaftEncoder::setPosition(long p) {
  encoderPosition = p;
}

//Retrieve count of invalid transitions
//unsigned ShaftEncoder::getNInvalids() {
//  return nInvalids;
//}

#endif    //ARDUINO_ARCH_AVR