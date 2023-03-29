/*
   @FILE Investigate use of interrupts with a two channel Analog to Digital Converter (ADC)

   Objectives:  Sample two analog signals (named, I and Q) using a periodic sample function

   Resources:  Two analog input pins

   Dependencies:  This implementation developed for an MCU having two ADC coverters, specifically
   the Adafruit Feather STM32F0405 Express.

   Implementation:  ADC0 is configured to run in continuous mode, sampling 

*/

//#if defined(ARDUINO_ARCH_STM32)

//Analog Wiring definitions
#define I_PIN A0  //Channel I on analog pin 0
#define Q_PIN A1  //Channel Q on analog pin 1



/**
   Initialization
*/
void setup() {
  Serial.begin(57600);
  Serial.print("\n\nStarting...\n");  
}

// Processor loop
void loop() {

  //Read pins
  int i = analogRead(I_PIN);
  int q = analogRead(Q_PIN);

}



