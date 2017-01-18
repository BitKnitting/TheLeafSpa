// Set DEBUG to 0 if NOT connected to serial monitor (for debugging)
#define DEBUG 1
#include <DebugLib.h>
// The Grove sensor is just a BoB using the DHT22
#include <DHT.h>
#define DHTPIN A0     // what pin we're connected to
// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
DHT dht(DHTPIN, DHTTYPE);
//1.Connect the Grove CO2 Sensor to Grove Base shield D7 Port
#include <SoftwareSerial.h>
SoftwareSerial CO2sensor(7, 8);      // TX, RX
const unsigned char cmdGetCO2Reading[] =
{
  0xff, 0x01, 0x86, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x79
};
// The GitHub location for the Timer library is here: //http://github.com/JChristensen/Timer
#include <Timer.h>
Timer tReadEvent;
// EEPROM is used to load/save global settings.
#define eepromWriteCheck 0x1234
#include <avr/eeprom.h>
struct globalSettings_T
{
  unsigned int writeCheck;
  int32_t secsBtwnReadings;
  int16_t CO2Level;
} globalSettings;
struct sensorData_T
{
  int CO2Value;
  float temperatureValue;
  float humidityValue;
} sensorData;

/*
   SETUP
*/
void setup() {
  DEBUG_BEGIN;
  DEBUG_PRINTF("The amount of available ram: ");
  DEBUG_PRINTLN(freeRam());
  //
  loadGlobalSettings();
  DEBUG_PRINTF("Seconds Between Readings: ");
  DEBUG_PRINT(globalSettings.secsBtwnReadings);
  DEBUG_PRINTF(" | CO2 Level: ");
  DEBUG_PRINTLN(globalSettings.CO2Level);
  //
  initSensors();
  tReadEvent.every(globalSettings.secsBtwnReadings, doReadings);
  doReadings();
}
/*
   LOOP
*/
void loop() {
  tReadEvent.update();
}
/*
   loadGlobalSettings() uses EEPROM to get the settings variables:
   - minsBtwnReadings
   - CO2Level
*/
void loadGlobalSettings() {
  DEBUG_PRINTLNF("In loadGlobalSettings()");
  eeprom_read_block(&globalSettings, (void *)0, sizeof(globalSettings));
  //if its a first time setup or our magic number in eeprom is wrong reset to default
  if (globalSettings.writeCheck != eepromWriteCheck) {
    resetGlobalSettings();
  }
}
/*
   resetGlobalSettings() variables to default values.
*/
void resetGlobalSettings() {
  DEBUG_PRINTLNF("In resetGlobalSettings()");
  globalSettings.writeCheck = eepromWriteCheck;
  globalSettings.secsBtwnReadings = (DEBUG == 1) ? 30 : 15 * 60;
  globalSettings.CO2Level = 1200;
}
/*
   initSensors() - initialize things needed to get the sensors working.
*/
void initSensors() {
  dht.begin();
  CO2sensor.begin(9600);
}
/*
   doReadings() is called every secsBtwnReadings. This is set up using the Timer.h functions.
   When doReadings() is invoked, read/display/send to Adafruit.io the temperature, humidity, and CO2 level.
*/
void doReadings() {
  DEBUG_PRINTLNF("in DoReadings()");
  sensorData.temperatureValue = dht.readTemperature();
  sensorData.humidityValue = dht.readHumidity();
  sensorData.CO2Value = takeCO2Reading();
  DEBUG_PRINTF("Temperature reading: ");
  DEBUG_PRINT(sensorData.temperatureValue);
  DEBUG_PRINTF("˚C | ");
  float farenheit = sensorData.temperatureValue * 1.8 + 32.;
  DEBUG_PRINT(farenheit);
  DEBUG_PRINTF("˚F | Humidity: ");
  DEBUG_PRINT(sensorData.humidityValue);
  DEBUG_PRINTF("% | CO2 reading: ");
  DEBUG_PRINT(sensorData.CO2Value);
  DEBUG_PRINTLNF(" PPM");
}
/*
   takeCO2Reading() - use the code from the GroveCO2ArduinoSketch to get CO2 data.
*/
int takeCO2Reading() {
  byte data[9];
  int i = 0;
  //transmit command data
  for (i = 0; i < sizeof(cmdGetCO2Reading); i++)
  {
    CO2sensor.write(cmdGetCO2Reading[i]);
  }
  delay(10);
  //begin receiving data
  if (CO2sensor.available())
  {
    while (CO2sensor.available())
    {
      for (int i = 0; i < 9; i++)
      {
        data[i] = CO2sensor.read();
      }
    }
  }
  if ((i != 9) || (1 + (0xFF ^ (byte)(data[1] + data[2] + data[3] + data[4] + data[5] + data[6] + data[7]))) != data[8])
  {
    return -1;
  }
  int CO2PPM = (int)data[2] * 256 + (int)data[3];
  //temperature = (int)data[4] - 40;  --> getting temperature from the DHT22.
  return CO2PPM;
}





