/*
   @FILE Investigate maximum interrupt frequency

  Objective:  Compare the count of processed digital interrupts with a known clock signal source

  Connections:  Connect the start button to the START_PIN, arranging for the button to pull the Open Collector
  digital input pin to ground when pressed.  Connect a TTL clock signal source to the SIGNAL_PIN.

  Operation:  When no measurement is in-progress and the start button is pressed, the investigation records the
  time when the new measurement period begins.  The signalISR counts the number of interrupts processed during
  the measurement period.  When the pre-determined MEASUREMENT_PERIOD ends, the code outputs the interrupt count
  to the serial port (typically a USB connection to a host computer).

  Usage:  With a 10 second period, the measured interrupt frequency is count/10.  When driven by a signal
  generator with an accurate clock, the investigator compares the measured interrupt frequency with the
  expected frequency.  If, for example, the apparatus is driven by a 100 kHz clock signal and the displayed
  count is 90000, then only 90% of the expected interrupts were actually processed by the apparatus.

  Notes:  The code ignores start button presses during a measurement interval, effectively de-bouncing the
  start button's noise.  Following initialization, the signal continuously interrupts the loop() and
  increments the count --- it will be reset to 0 when a subsequent measurement begins.
*/

//Define the Digital Pin Numbers for the button and signal
#define START_PIN 6      //Start measurement button
#define SIGNAL_PIN 7     //Interrupt source signal

//Define the measurement period in milliseconds
#define MEASUREMENT_PERIOD 10000L //Ten seconds

//State variables
volatile unsigned long count;     //Number of interrupts during a measurement period
volatile boolean startPressed;    //Indicates when the start of a new measurement is pending
unsigned long startTime;          //Records the time when a new measurement begins
boolean inProgress;               //Indicates when a measurement period is in progress

/**
   Start Button Interrupt Service Routine (ISR)
*/
void startButtonISR() {
  startPressed = true;      //Notifies the loop() when button has been pressed
}

/**
   Signal Interrupt Service Routine (ISR)
*/
void signalISR() {
  count++;                  //Increment count of signal interrupts
}


/**
   Initialization
*/
void setup() {

  //Setup the start button
  inProgress = false;                        //No measurement is in-progress
  startPressed = false;                      //Button has not yet been pressed
  pinMode(START_PIN, INPUT_PULLUP);          //Digital Input with a pullup resistor button will short to GND
  attachInterrupt(digitalPinToInterrupt(START_PIN), startButtonISR, FALLING); //Interrupt when button pressed

  //Setup the signal source to be measured
  pinMode(SIGNAL_PIN, INPUT);                //Pin will be driven by an external TTL clock source
  attachInterrupt(digitalPinToInterrupt(SIGNAL_PIN), signalISR, RISING); //Interrupt on rising clock sourcde pulse

}

void loop() {

  //Is an inProgress measurement ending?
  if (inProgress && ( (millis() - startTime) >= MEASUREMENT_PERIOD)) {
    printf("count = %ul\n", count);         //Output the count of signal interrrupts over serial port
    inProgress = false;                     //Measurement has finished
    startPressed = false;                   //Prepare for a subsequent measurement
  }

  //Check for startButton pressed to start a measurement
  if (!inProgress && startPressed) {
    startPressed = false;                   //Reset the button state
    startTime = millis();                   //Record time when measurement period begins
    count = 0;                              //Reset count of signal interrupt signals
    inProgress = true;                      //Note that a measurement is in-progress
    printf("starting\n");                   //Notify the investigator (might this interfere with signal interrupts???)
  }

}
