/*
   @FILE Investigate performance of the parallel LCD library functions

   Connections:
     RS_PIN - The Arduino Digital pin connected to the LCD's RS signal
     ENABLE_PIN - The Arduino Digital pin connected to the LCD's ENABLE signal
     D4_PIN - The Arduino Digital pin connected to the LCD's D4 signal
     D5_PIN - The Arduino Digital pin connected to the LCD's D5 signal
     D6_PIN - The Arduino Digital pin connected to the LCD's D6 signal
     D7_PIN - The Arduino Digital pin connected to the LCD's D7 signal

   Usage:  No user interaction is required.  The program exercises the timing of various
   LCD functions and outputs their results to the serial (USB) port.

   Limitations:  Exercises only parallel (e.g. Hitachi HD44780 interface) text-based
   LCDs.  The code as implemented supports only the 4-wire data connection and
   assumes the LCD has at least 2 rows of at least 16 characters.

*/

#include <LiquidCrystal.h>
#include <ShaftEncoder.h>

//Build the lcd object
LiquidCrystal lcd(12, 11, 10, 5, 4, 3, 2);

//Build the ShaftEncoder
ShaftEncoder encoder(6,7,false);    //No interrupts available on arduino digital pins 6 and 7

//Iteration counter
unsigned long iterationCount;


/**
   Initialization
*/
void setup() {

  Serial.begin(57600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.print("Starting...");

  //Display the starting message on the LCD to exercise the connection
  lcd.begin(16, 2);             //Two rows of sixteen characters each
  lcd.print("Starting!");       //Display the starting message
  delay(1000);                  //Wait a second

  //Reset iteration counter
  iterationCount=0;

}

void loop() {

  //Display shaft encoder's position in first row
  long x = ShaftEncoder::getPosition();
  lcd.setCursor(0,0);           //First column, first row
  lcd.print(x);

  //Display updated iteration count in second row
  lcd.setCursor(0,1);           //First column, second row
  lcd.print(++iterationCount);

}
