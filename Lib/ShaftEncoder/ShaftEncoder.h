/**
   @file Definitions for all implementations of the ShaftEncoder
*/

class ShaftEncoder {

public:
  ShaftEncoder(uint8_t a, uint8_t b, bool interruptEnable);
  static void encoderSetup();
  static long getPosition();
  static void setPosition(long);
//static unsigned getNInvalids();
};