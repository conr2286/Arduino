/*
   @FILE Investigate performance of a text-based OLED library's functions

   Connections:
   Connect the OLED device to the Arduino's I2C bus

   Usage:  No user interaction is required.  The program exercises the timing of various
   OLED functions and outputs their results to the serial (USB) port.

*/

#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels




//Define the OLED's address on the I2C bus (must match the OLED's specs)
#define OLED_I2C_ADDRESS 0x3C

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

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

  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!oled.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  oled.clearDisplay();
  oled.print("Starting...");

  //Time various OLED functions of interest
  timeIt("setCursor", exerciseSetCursor);
  timeIt("print", exercisePrint);
  timeIt("write", exerciseWrite);

}

void loop() {

}
