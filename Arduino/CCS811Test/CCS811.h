#include <Wire.h>
#include "i2cHelpers.h"

class CCS811Arduino {
  private:
    int wakePin;
    //    // From the datasheet:
    //    // When I2C _ADDR is low the 7 bit I2C address is decimal 90 / hex 0x5A
    //    // When I2C _ADDR is high the 7 bit I2C address is decimal 91 / hex 0x5B
    //    // I currently have the BoB to use 0x5B - JP1 not attached
    const int  CCS811i2cAddress =                0x5B;
    const int  CCS811_hwID =                     0x20;
  public:
    byte errCode;
    CCS811Arduino (int pin) {
      wakePin = pin;
      //From the datasheet: nWAKE is an active low input and should be asserted by the host
      //prior to an I2C transaction and held low throughout.
      pinMode(wakePin, OUTPUT);
      digitalWrite(wakePin, LOW);
    }
    init() {
      errCode = initI2C(CCS811i2cAddress);
    }
    void gethwID(byte *hwID) {
      Wire.beginTransmission(CCS811i2cAddress);
      Wire.write(CCS811_hwID);
      errCode = Wire.endTransmission();
      if (errCode) return;
      Wire.requestFrom(CCS811i2cAddress, 1);
      if (Wire.available() == 1) {
        *hwID = Wire.read();
      }
    }
};

