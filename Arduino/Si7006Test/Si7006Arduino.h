#include <Wire.h>
#include "i2cHelpers.h"

class Si7006 {
  private:
    const int Si7006i2cAddress =                0x40;
    const int Si7006_FIRMWARE_0 =               0x84;
    const int Si7006_FIRMWARE_1 =               0xB8;
    // Master mode works.  No Master mode does not work.
    const int Si7006_HUMIDITY_MASTER_MODE =     0xE5;
    const int Si7006_TEMPERATURE  =             0xE0;
  public:
    byte errCode;
    void init() {
      errCode = initI2C(Si7006i2cAddress);
    }
    /* getFirmwareRevision(byte *firmware)
      the referenced byte -> firmware <- is the return value that is the firmware revision.
      the errCode is returned in the case the I2C communication fails.
      The Firmware revision is documented in the data sheet.
      If the byte returned is 0xFF, the firmware is version 1.0.
      If the byte returned in 0X20, the firmware is version 2.0.
    */
    void getFirmwareRevision(byte *firmware) {
      Wire.beginTransmission(Si7006i2cAddress);
      Wire.write(Si7006_FIRMWARE_0);
      Wire.write(Si7006_FIRMWARE_1);
      errCode = Wire.endTransmission();
      if (errCode) return;
      Wire.requestFrom(Si7006i2cAddress, 1);
      if (Wire.available() == 1) {
        *firmware = Wire.read();
      }
    }
    /*
      getHumidityAndTemperature() puts the relative humidity in % and the temperature in ˚C  within
      parameters humidity and temperature.
      See errors.h for a discussion of the error codes that can be returned.
      The temperature reading uses the technique in the datasheet that gets the bytes for temperature from the
      humidity reading.  From the datasheet:
      Each time a relative humidity measurement is made a temperature measurement is also made
      for the purposes of temperature compensation of the relative humidity measurement. If the
      temperature value is required, it can be read using command 0xE0; this avoids having to
      perform a second temperature measurement.

    */
    void getHumidityAndTemperature(float &humidity, float &temperature) {
      byte high, low;
      unsigned int value;
      Wire.beginTransmission(Si7006i2cAddress);
      Wire.write(Si7006_HUMIDITY_MASTER_MODE);
      errCode = Wire.endTransmission();
      if (errCode) return;
      errCode = getUInt(value);
      if (errCode) return;
      //From datasheet: A humidity measurement will always return XXXXXX10 in the LSB field.
      if (value & 0xFFFE) {
        humidity = ((125 * (float)value ) / 65536) - 6;
      } else {
        errCode = errorSi7006Humidity;
        return ;
      }
      //onto the temperature.
      Wire.beginTransmission(Si7006i2cAddress);
      Wire.write(Si7006_TEMPERATURE);
      errCode = Wire.endTransmission();
      if (errCode) return;
      errCode = getUInt(value);
      if (errCode) return;
      // A temperature measurement will always return XXXXXX00 in the LSB field.
      if (value & 0xFFFC) {
        temperature = (172.72 * (float)value) / 65536 - 46.85;
      } else {
        errCode = errorSi7006Temperature;
        return;
      }
    }
};



