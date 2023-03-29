/*
   @FILE Investigate performance of  digital output

   Objective:  Measure how quickly the Arduino core can toggle a digital output pin

   Connections:  Connect an oscilloscope to the SIGNAL_PIN to measure the output pulse width

*/

#include <SoftwareSerial.h>

//Define the Digital Pin Numbers for the button and signal
#define SIGNAL_PIN 7     //Interrupt source signal




/**
   Initialization
*/
void setup() {
  Serial.begin(57600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.print("Starting...\n");

  //Setup the signal output
  pinMode(SIGNAL_PIN, OUTPUT);                //Pin will be driven by our code
  digitalWrite(SIGNAL_PIN, LOW);              //Initially at Logic 0




}

void loop() {

  //Toggle the pin as quickly as possible
  digitalWrite(SIGNAL_PIN, HIGH);           //Logic 1
  digitalWrite(SIGNAL_PIN, LOW);            //Logic 0

}
