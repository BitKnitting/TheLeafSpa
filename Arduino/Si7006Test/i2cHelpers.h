#ifndef I2CHELPERS_H
#define I2CHELPERS_H
#include "errors.h"
int i2cAddress;
/*
   initI2C()
   check to see if SDA and SCL lines are high - This check can be useful mostly in the wiring from
   the Arduino to the BoB where the i2c chip is soldered.  It could also mean something is not right
   with the pull up resistor on the SDA and SCL tracks. 
   NOTE: Assumes SDA and SCL are wired to pins A4 and A5...i.e.: standard I2C pins on Arduino Uno....
*/
byte initI2C(int addr) {
  i2cAddress = addr;
  if ((digitalRead(A4) == HIGH) && (digitalRead(A5) == HIGH)) {
    Wire.begin();
    return errorSuccess;
  }
  return errorI2cBegin;
}
/*
   getUInt() returns a 0 if successfully read two bytes from I2C and put into the value parameter passed in.
   if not successful getting the two bytes, a 1 is returned (as an error code).
*/
byte getUInt(unsigned int &value) {
  byte high, low;
  Wire.requestFrom(i2cAddress, 2);
  if (Wire.available() == 2) {
    high = Wire.read();
    low = Wire.read();
    value = word(high, low);
    return errorSuccess;
  }
  return errorI2cHelpersReadUInt;
}
#endif
