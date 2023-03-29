/**
  @file Investigate the GPIO architectures

  Usage:  The program configures TEST_PIN for digital input with a pullup resistor, and
  then loops endlessly, displaying the current count of interrupts arising from rising-edges
  on that pin.

  @author KQ7B --- Jim Conrad
**/


#include "GPIOAbstractor.h"

#define TEST_PIN 5

#ifdef GPIO_DEBUG
extern volatile unsigned nPortB, nPortC, nPortD;
#endif


typedef unsigned long u32;

GPIOAbstractor gpio;

volatile u32 count;
u32 stopTime;               //milliseconds
const u32 duration=1000L;   //milliseconds

void myISR() {
  count++;
}

void setup() {
  count = 0;
  stopTime = millis() + 1000L;
  Serial.begin(57600);
  Serial.println("\nStarting...");

  //Prevent interrupts asrising from electrical noise on unused pins.
  for (int i=0; i<23; i++) pinMode(i,INPUT_PULLUP);

  //Configure test pin and attach an interrupt handler for it
  gpio.pinMode(TEST_PIN,INPUT_PULLUP);
  gpio.attachInterrupt(TEST_PIN, myISR, RISING);
}

void loop() {
  if (millis() >= stopTime) {
    Serial.print("freq="); Serial.print(count);          //Display count
#ifdef GPIO_DEBUG
    Serial.print(", nPortB="); Serial.print(nPortB);      //Interrupt counts on ports
    Serial.print(", nPortC="); Serial.print(nPortC);      //Interrupt counts on ports
    Serial.print(", nPortD="); Serial.print(nPortD);      //Interrupt counts on ports
#endif
    Serial.print("\n");
    count = 0;                    //Reset count to see frequency
    stopTime = millis() + duration;
  }
}
