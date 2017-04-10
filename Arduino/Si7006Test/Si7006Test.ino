#include "Si7006Arduino.h"
#include "errorMessages.h"
char errorBuffer[50] = {0};
// instantiate an instance of an Si7006.  I.e.: the instance is code that talks to an Si7006 providing ways to get to temperature and humidity readings.  The
// firmware revision is included as another function. The only functionality really needed by the Leaf Spa is getting the temperature and humidity.
Si7006 si7006;
void setup() {
  Serial.begin(115200);
  Serial.println(F("********Si7006 example sketch********"));
  si7006.init();
  // The returned firmware version is stored in a byte.
  byte firmware;
  si7006.getFirmwareRevision(&firmware);
  if (!si7006.errCode) {
    Serial.print(F("Firmware Revision: 0X"));
    Serial.println(firmware, HEX);
  } else {
    errorMessage(si7006.errCode, errorBuffer);
    Serial.println(errorBuffer);
  }
  //The returned values for humidity and temperature are stored as floats.
  float humidity, temperature;
  si7006.getHumidityAndTemperature(humidity, temperature);
  if (!si7006.errCode) {
    Serial.print(F("Humidity: "));
    Serial.print(humidity);
    Serial.println(F("%"));
    Serial.print(F("Temperature: "));
    Serial.print(temperature);
    Serial.print(F("˚C, "));
    temperature = temperature * 9/5 + 32;
    Serial.print(temperature);
    Serial.println(F("˚F "));
  } else {
    errorMessage(si7006.errCode, errorBuffer);
    Serial.println(errorBuffer);
  }
}
void loop() {
}


