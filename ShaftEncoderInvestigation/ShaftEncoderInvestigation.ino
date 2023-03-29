/**
 * @file
 * @author  Jim Conrad, KQ7B
 * 
 * Investigate the operation of the Sparkfun 200 P/R Rotary Encoder (COM-10932) interfaced to an Arduino board
 * 
 * The COM-10932 encoder outputs gray code, two digital signals, here referred to as ENCODER_A and ENCODER_B, that can be decoded
 * to determine which direction and how far its shaft has been rotated.  The outputs are NPN open collector (3..12V) and
 * this implementation arranges for the Arduino to provide the pullup resistors.
 *  
 */

//Install the ShaftEncoder library zip from below the lib folder
#include <ShaftEncoder.h>

//Define encoder pin connections to the Arduino
#define ENCODER_A 2  //ENCODER_A is Arduino Digital 2 (PD2) appearing as Pin 5 on the Arduino card
#define ENCODER_B 3  //ENCODER_B is Arduino Digital 3 (PD3) appearing as Pin 6 on the Arduino card


//Build the ShaftEncoder.  Without interrupts, the encoder's position must be polled *very* rapidly
//to avoid missing the encoders transitions
#define INTEN false
ShaftEncoder encoder(ENCODER_A, ENCODER_B, INTEN);

//ShaftEncoder encoder(ENCODER_A, ENCODER_B, true);

//Initialization
void setup() {
  Serial.begin(57600);
  Serial.print("\nsetup()\n");
}


//The main loop, invoked ad infinatum as fast as the Arduino core can do it
void loop() {
  Serial.print("\nencoderPosition="); Serial.print(ShaftEncoder::getPosition(),DEC);
  delay(1);  //Even a 1 mS delay can lead to missing transitions w/o interrupts
}
