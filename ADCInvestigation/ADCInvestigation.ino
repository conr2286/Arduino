/*
   @FILE Investigate use of interrupts with the Analog to Digital Converter (ADC)

   Objectives:  Start the ADC and allow it to operate concurrently with the CPU.  Operate
   the ADC at a known sample rate.  Interrupt the CPU when a conversion finishes, fetch
   the result, and store it in a buffer.

   Dependencies:  As currently written, this code supports the ATmega328 and equivalent
   processors.   

   Connections:  Connect a signal source to the ADC_PIN.  The source frequency should not
   exceed 0.5*sampleFrequency (Nyquist criteria).
*/

#if defined(ARDUINO_ARCH_AVR)

//Analog Wiring definitions
#define ADC_PIN A0  //Arduino Analog Input Pin 0

//ATmega analog reference sources
#define REF_ADREF 0b00000000  //Reference the AREF pin in range 1...Vcc
#define REF_AVCC 0b01000000   //Reference AVCC (i.e. Vcc)
#define REF_11 0b11000000     //Use the internal 1.1V reference

//ATmega result bit alignment
#define D_ADLAR 0b00000000  //Right adjusted for 10-bit result

//ATmega analog MUX mask
#define D_AMUXMSK 0b00001111  //The rightmost nibble

//ATmega ADC Control and Status Register A
#define D_ADEN 0b10000000     //Enable the ADC
#define D_ADSC 0b01000000     //Start conversion (can be polled to determine completion)
#define D_ADATE 0b00100000    //Enable auto triggering
#define D_ADIE 0b00001000     //Interrupt enable (through ADC_vec)
#define D_ADPSMSK 0b00000111  //Clock pre-scaler mask

//ATmega ADC prescaler values determine ADC clock frequency
#define D_ADPS2 0b00000001    //FCPU/2
#define D_ADPS4 0b00000010    //FCPU/4
#define D_ADPS8 0b00000011    //FCPU/8
#define D_ADPS16 0b00000100   //FCPU/16
#define D_ADPS32 0b00000101   //FCPU/32
#define D_ADPS64 0b00000110   //FCPU/64
#define D_ADPS128 0b00000111  //FCPU/128


//True when ADC completes a conversion
volatile bool adcReady;

//The ADC result
volatile int adcVal;


/**
   Initialization
*/
void setup() {
  Serial.begin(57600);
  pinMode(7, OUTPUT);  //Using PD7 to signal conversion completion
  Serial.print("\n\nADCInvestigation Starting...\n");

  //Configure the ADMUX fields
  ADMUX &= 0b11011111;  //Right-adjust 10-bit conversion results
  ADMUX |= 0b01000000;  //Use Vcc for the analog reference source
  ADMUX &= 0b11110000;  //Select Arduino pin A0 with multiplexor
  ADMUX |= 0;           //Analog pin 0 (A0)
  Serial.print("ADMUX = ");
  Serial.print(ADMUX, BIN);

  //Configure ADC's ADCSRA and ADCSRB fields
  ADCSRA |= 0b10000000;  //Enable this ADC
  ADCSRA |= 0b00100000;  //Enable auto-triggering for this ADC...
  ADCSRB &= 0b11111000;  //...and sample continuously
  ADCSRA |= 0b00000111;  //Prescaler will divide FCPU by 128
  ADCSRA |= 0b00001000;  //Enable ADC interrupts
  Serial.print("   ADCSRA = ");
  Serial.print(ADCSRA, BIN);

  //Start the ADC
  adcReady = false;
  ADCSRA |= B01000000;  //Start the first conversion
}

// Processor loop
void loop() {

  // Has the ADC completed a conversion?
  if (adcReady) {

    adcReady = false;  //Yes, reset the flag

    //Quickly pulse Digital Pin 7
    PORTD = B10000000;
    PORTD = B00000000;
  }
}




// Interrupt service routine for the ADC completion
ISR(ADC_vect) {

  // Done reading
  adcReady = true;

  // Assemble the result, reading the low-order byte first
  adcVal = ADCL | (ADCH << 8);
}

#endif