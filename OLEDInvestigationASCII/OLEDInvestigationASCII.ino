/*
   @FILE Investigate performance of a text-based OLED library's functions

   Connections:
   Connect the OLED device to the Arduino's I2C bus

   Usage:  No user interaction is required.  The program exercises the timing of various
   OLED functions and outputs their results to the serial (USB) port.

*/

#include <Wire.h>

#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"



//Define the OLED's address on the I2C bus (must match the OLED's specs)
#define OLED_I2C_ADDRESS 0x3C

//Build the OLED object
SSD1306AsciiWire oled;

/*
   Time the performance of a single feature
*/
void timeIt( char *s, void (*theFunction)() ) {
  unsigned repetitions = 10000;
  unsigned long t0 = millis();        //Time when exercise begins
  for (unsigned i = 0; i < repetitions; i++) {
    theFunction();
  }
  unsigned long t1 = millis();        //Time when exercise ends
  printf("%s %f ms\n", s, ((float)(t1 - t0) / (float)repetitions));
}

/**
   exerciseSetCursor
*/
unsigned row = 0;
unsigned col = 0;
void exerciseSetCursor() {
  oled.setCursor(col, row);
  row = (++row) % 2;
  col = (++col) % 16;
}

/**
   exercisePrint
*/
void exercisePrint() {
  oled.print("p");
}

/**
 * exerciseWrite()
 */
 void exerciseWrite() {
  oled.write('w');
 }

/**
   Initialization
*/
void setup() {

  Wire.begin();
  Wire.setClock(400000L);

  oled.begin(&Adafruit128x64, OLED_I2C_ADDRESS);
  oled.setFont(System5x7);
  oled.clear();
  oled.print("Starting...");

  //Time various OLED functions of interest
  timeIt("setCursor", exerciseSetCursor);
  timeIt("print", exercisePrint);
  timeIt("write", exerciseWrite);

}

void loop() {

}
