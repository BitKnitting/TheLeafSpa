// Set DEBUG to 0 if NOT connected to serial monitor (for debugging)
#define DEBUG 1
#include <DebugLib.h>
// Set TEST_ACTIONS to 1 if testing how the on/off actions work.
#define TEST_ACTIONS 1
/* The SD code I got from the SD card file dump - I got from somewhere. From the file.....

  created  22 December 2010  by Limor Fried
  modified 9 Apr 2012  by Tom Igoe

  This example code is in the public domain.

*/
#include <SPI.h>
#include <SD.h>
//     Adafruit SD shields and modules: pin 10
const int chipSelect = 10;
#include <DHT.h>
#define DHTPIN 5     // what pin we're connected to
// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
DHT dht(DHTPIN, DHTTYPE);
#include <SoftwareSerial.h>
SoftwareSerial CO2sensor(6, 7);   // TX, RX
const unsigned char cmdGetCO2Reading[] =
{
  0xff, 0x01, 0x86, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x79
};
// The GitHub location for the Timer library is here: //http://github.com/JChristensen/Timer
#include <Timer.h>
Timer timer;
// RELAY pins
#define ON LOW
#define OFF HIGH
#define pumpPin 4 //put the DC pin for the relay that will control the pump into pin 4 of the Arduino.
#define LEDPin  5
// EEPROM is used to load/save global settings.
#define eepromWriteCheck 0x5678
#include <avr/eeprom.h>
struct globalSettings_T
{
  unsigned int writeCheck;
  int32_t            secsBtwnReadings;
  int16_t            targetCO2Level;
  int16_t            amtSecsWaterPumpIsOn;
  unsigned long      secsBetweenTurningPumpON;
  int16_t            photoperiod;
} globalSettings;
struct sensorData_T
{
  int CO2Value;
  float temperatureValue;
  float humidityValue;
} sensorData;
bool fLEDsAreOn = false;
/*
   SETUP
*/
void setup() {
  DEBUG_BEGIN;
  DEBUG_PRINTF("The amount of available ram: ");
  DEBUG_PRINTLN(freeRam());
  //
  loadGlobalSettings();
  debugPrintGlobalSettings();
  initStuff();
}
/*
   LOOP
*/
void loop() {
  timer.update();
}
/*
   loadGlobalSettings() uses EEPROM to get the settings variables identified within the globalSettings structure.
   if the write check determines the settings are either not there or the write check itself is corrupt, the settings
   are reset to default values.
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
   resetGlobalSettings() variables to default values.  See LoadGlobalSettings() to get an idea when this function is called.
*/
void resetGlobalSettings() {
  DEBUG_PRINTLNF("In resetGlobalSettings()");
  globalSettings.writeCheck = eepromWriteCheck;
  globalSettings.secsBtwnReadings = (DEBUG == 1) ? 2 : 15 * 60;  //if in debug mode, make the period between readings short.
  globalSettings.targetCO2Level = 1200;
  globalSettings.amtSecsWaterPumpIsOn = (DEBUG == 1) ? 3 :  60; //amount of seconds for pump to be ON.
  globalSettings.secsBetweenTurningPumpON = (DEBUG == 1) ? 20 :  60 * 30; //# mins between turning pump ON.
  globalSettings.photoperiod = 20; //LED is off 4 hours of the 24 hours.
}
/*
   print out the values that will be used to control the environment (GlobalSettings) when DEBUG is on.
*/
void debugPrintGlobalSettings() {
  DEBUG_PRINTF("Seconds Between Readings: ");
  DEBUG_PRINT(globalSettings.secsBtwnReadings);
  DEBUG_PRINTF(" | Target CO2 Level: ");
  DEBUG_PRINTLN(globalSettings.targetCO2Level);
  DEBUG_PRINTF(" | # secs water pump will be on: ");
  DEBUG_PRINTLN(globalSettings.amtSecsWaterPumpIsOn);
  DEBUG_PRINTF(" | Seconds between turning the pump ON: ");
  DEBUG_PRINTLN(globalSettings.secsBetweenTurningPumpON);
  DEBUG_PRINTF(" | Photoperiod: ");
  DEBUG_PRINTLN(  globalSettings.photoperiod);
}
/*
   initStuff() - initialize things needed to get sensors and logging working.
*/
void initStuff() {
  initSD();
  dht.begin();
  CO2sensor.begin(9600);
  pinMode(pumpPin, OUTPUT);
  pinMode(LEDPin,OUTPUT);
  digitalWrite(pumpPin, OFF);
  digitalWrite(LEDPin,OFF);
  DEBUG_PRINTF("Setting timer to read sensors every ");
  DEBUG_PRINT(globalSettings.secsBtwnReadings);
  DEBUG_PRINTLNF(" seconds.");
  //set up reading the sensors
  timer.every(globalSettings.secsBtwnReadings * 1000, doReading); //set the timer up to go off when it is time to take sensor readings.
  doReading();
  // set up turning the pump off and on.
  timer.every(globalSettings.secsBetweenTurningPumpON * 1000, doPump);
  doPump();
  //set up LED photoperiod
}
/*
   Initialize the SD card for logging
*/
bool initSD() {
  if (TEST_ACTIONS) {
    DEBUG_PRINTLNF("Not writing to the SD card in this test. ");
  } else {
    DEBUG_PRINTF("Initializing SD card...");
    if (!SD.begin(chipSelect)) {
      DEBUG_PRINTLNF("initialization failed. Things to check:");
      DEBUG_PRINTLNF("* is a card inserted?");
      DEBUG_PRINTLNF("* is your wiring correct?");
      DEBUG_PRINTLNF("* did you change the chipSelect pin to match your shield or module?");
      return false;
    }
  }
  return true;
}
/*
   doReading() is called every secsBtwnReadings (see tReadEvent.every(...doReading)... This is set up using the Timer.h functions.
   When doReading() is invoked, read/log/display(?) sensor data.  Also determine if an action (like adjusting the CO2 level or
   turning the LED lighting on/off) should happen.
*/
void doReading() {
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
  //TBD: Write to log file...
}
/*
   doPump() turn the water pump on off
*/
void doPump() {
  //TBD: Log that pump was turned on for amtSecsWaterPumpIsOn
  //TBD: Use a clock?
  timer.pulse(pumpPin, globalSettings.amtSecsWaterPumpIsOn * 1000, OFF);
  DEBUG_PRINTF(" Pump ON for ");
  DEBUG_PRINT(globalSettings.amtSecsWaterPumpIsOn);
  DEBUG_PRINTLNF(" seconds.");
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
  //check to see if CO2 regulator needs to be turned on
  adjustCO2(CO2PPM);
  return CO2PPM;
}
void adjustCO2(int co2reading) {
  if (fLEDsAreOn) {
    
  }
  
}

