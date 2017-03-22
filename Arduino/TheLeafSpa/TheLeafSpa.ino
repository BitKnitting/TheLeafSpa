// Comment out #define DEBUG if not in debug mode
//#define DEBUG 
#include <DebugLib.h>
/*******************************************************************
 * Arduino Firmware for The Leaf Spa.  I have been documenting this
 * project on the bitknitting blog.  The intent is to control variables such
 * as CO2 level, photoperiod, providing nutrients as well as monitor variables
 * such as temperature, humidity, and CO2.
 * 
 * If you find the code useful, it would be awesome
 */


#include <SPI.h>
#include <SD.h>
//     Adafruit SD shields and modules: pin 10
const int chipSelect = 10;
const int cardDetectPin = 3;
enum cardState_t {
  Inserted = LOW,
  Removed = HIGH,
  Unchanged = HIGH + 1,
  Initial = HIGH + 2
} prevCardState, currentCardState;
/******************************************************************
   PIN MAP
   2 - DHT temp/humidity
   3 - SD card detect
   4 - pump relay
   5 - LED relay
   6 - CO2 relay
   7 - Software Serial
   8 - Software Serial
   10 - SDI chip select
   11 - SDI DI pin
   12 - SDI DO pin
   13 - SDI CLK pin
 *******************************************************************/
#include <DHT.h>
#define DHTPIN 2
// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
DHT dht(DHTPIN, DHTTYPE);
#include <SoftwareSerial.h>
SoftwareSerial CO2sensor(7, 8); // TX, RX
const unsigned char cmdGetCO2Reading[] =
{
  0xff, 0x01, 0x86, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x79
};
//RTC timer functionality.  Refer to the discussion on this blog post: https://bitknitting.wordpress.com/2017/02/10/build-log-for-february-9th/
#include <TimeLib.h>
#include <TimeAlarms.h>
#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t
// RELAY pins
#define ON LOW //I discuss relay ON / OFF in this blog post: https://bitknitting.wordpress.com/2017/02/03/build-log-for-february-2nd/
#define OFF HIGH
#define pumpPin 4 //put the pin for the relay that will control the pump into pin 4 of the Arduino.  Make sure the pump is plugged into the right socket.
#define LEDPin  5 //same thing as for the pumpPin...
#define CO2Pin  6 //same things as for the other pins...
//Use a flag to tell if the light is on.  Knowing if the light is on is important for adjusting CO2.
bool fLEDon = false;
//Define a warm up time for the CO2 sensor.  The Grove wiki says 3 minutes: https://seeeddoc.github.io/Grove-CO2_Sensor/
const int secsWarmUp = 3 * 60;
//Use a flag to tell the code when the warm up period is over so that readings can be taken and logged.
bool fInWarmUp = true;
// EEPROM is used to load/save global settings.  See resetGlobalSettings() to get a feel for what properties are stored.
#ifdef DEBUG
#define eepromWriteCheck 0x5678
#else
#define eepromWriteCheck 0x1234
#endif
#include <avr/eeprom.h>
struct globalSettingsV1_T
{
  unsigned int writeCheck;
  int32_t            secsBtwnReadings;
  int16_t            targetCO2Level;
  int16_t            amtSecsWaterPumpIsOn;
  unsigned long      secsBetweenTurningPumpON;
  int                hourToTurnLightOff;
  int                hourToTurnLightOn;
  time_t             timeCardWasRemoved;
} globalSettings;
struct sensorData_T
{
  int CO2Value;
  float temperatureValue;
  float humidityValue;
} sensorData;
//The log file holds rows that are different types of data depending on what the activity is
//reading the sensors, turning the pump, LED, or CO2 on/off
enum logRow_t {
  SensorData,   //0
  PumpOn,       //1
  PumpOff,      //2
  LEDOn,        //3
  LEDOff,       //4
  CO2On,        //5
  CO2Off,       //6
  CardInserted, //7
  CardRemoved,  //8
  WarmupStart,  //9
  WarmupEnd,     //10
  Settings_V1 = 51 //51 will be the number used to identify the logfile contains sensor readings and actions
                //based on globalSettingsV1_T variables
} ;
const char *logFileName = "datalog.txt";

/***********************************************************
   setup()
 ***********************************************************/
void setup() {
  initStuff();
}
/***********************************************************
   loop()
 ***********************************************************/
void loop() {
  //The Alarm callbacks were not called unless Alarm.Delay() was in the loop.
  Alarm.delay(1);
}
/*
   initStuff()
*/
void initStuff() {
  DEBUG_BEGIN;
  DEBUG_WAIT;
  DEBUG_PRINTF("\nThe amount of available ram: ");
  DEBUG_PRINT(freeRam());
  DEBUG_PRINTF(" | The amount of available SRAM: ");
  DEBUG_PRINTLN(availableMemory());
  setSyncProvider(RTC.get);   // the function to sync the time from the RTC..from Paul's example.
  if (timeStatus() != timeSet)
    DEBUG_PRINTLNF("Unable to sync with the RTC");
  else
    DEBUG_PRINTLNF("RTC has set the system time");
  dht.begin();
  CO2sensor.begin(9600);
  // The cardDetectPin is used to figure out if the SD card is in the slot...
  // I'm using INPUT_PULLUP for the pin mode to use attachInterupt()...the challenge with
  // attachInterupt() inserting/removing card typically generates more than 1 interrupt so
  // it is difficult to know what the last one is....
  pinMode(cardDetectPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(cardDetectPin), closeSD, CHANGE);
  //call in init to set whether the SD Card was in the reader when script started (or not).
  prevCardState = Initial;
  DEBUG_PRINTF("Previous card state on init: ");
  DEBUG_PRINTLN(prevCardState);
  pinMode(pumpPin, OUTPUT);
  pinMode(LEDPin, OUTPUT);
  pinMode(CO2Pin, OUTPUT);
  digitalWrite(pumpPin, OFF);
  digitalWrite(LEDPin, OFF);
  digitalWrite(CO2Pin, OFF);
  loadGlobalSettings();
  debugPrintGlobalSettings();
  DEBUG_PRINTF("Setting timer to read sensors every ");
  DEBUG_PRINT(globalSettings.secsBtwnReadings);
  DEBUG_PRINTLNF(" seconds.");
  // Set up reading the sensors.
  Alarm.timerRepeat((const int)globalSettings.secsBtwnReadings, doReading); //set the timer up to go off when it is time to take sensor readings.
  // Set up turning the pump off and on.
  Alarm.timerRepeat((const int)globalSettings.secsBetweenTurningPumpON, doPump);
  // Set up LED photoperiod through on and off alarms that fire every day.
  Alarm.alarmRepeat((const int)globalSettings.hourToTurnLightOff, 0, 0, turnLightOff);
  Alarm.alarmRepeat((const int)globalSettings.hourToTurnLightOn, 0, 0, turnLightOn);
  // Set a timer for the number of minutes needed to warm up the CO2 sensor before taking readings
  fInWarmUp = true;
  DEBUG_PRINTLNF("Warming up.");
  writeEventHappened(WarmupStart);
  Alarm.timerOnce((const unsigned long)secsWarmUp, warmUpOver);
}
/*
   loadGlobalSettings() uses EEPROM to get the settings variables identified within the globalSettings structure.
   if the write check determines the settings are either not there or the write check itself is corrupt, the settings
   are reset to default values.'
*/
void loadGlobalSettings() {
  DEBUG_PRINTLNF("In loadGlobalSettings()");
  eeprom_read_block(&globalSettings, (void *)0, sizeof(globalSettings));
  //if its a first time setup or our magic number in eeprom is wrong reset to default
  if (globalSettings.writeCheck != eepromWriteCheck) {
    resetGlobalSettings();
  }
  writeSettingsVersionToLogFile();
}
/*
   resetGlobalSettings() variables to default values.  See LoadGlobalSettings() to get an idea when this function is called.
*/
void resetGlobalSettings() {
  bool debug = true;
#ifndef DEBUG
  debug = false;
#endif
  DEBUG_PRINTLNF("In resetGlobalSettings()");
  globalSettings.writeCheck = eepromWriteCheck;
  globalSettings.secsBtwnReadings = (debug == 1) ? 60 : 2 * 60;  //if in debug mode, make the period between readings short.
  globalSettings.targetCO2Level = 1200;
  globalSettings.amtSecsWaterPumpIsOn = (debug == 1) ? 5 :  60; //amount of seconds for pump to be ON.
  globalSettings.secsBetweenTurningPumpON = (debug == 1) ? 60 :  15 * 60; //# secs between turning pump ON.
  globalSettings.hourToTurnLightOff = 0; //Turn light off at midnight.
  globalSettings.hourToTurnLightOn = 8; //Turn light on at 8AM. (16 hour daylight)
  eeprom_write_block(&globalSettings, (void *)0, sizeof(globalSettings)); //write settings to eeprom
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
  DEBUG_PRINTF(" | Hour to turn light OFF: ");
  DEBUG_PRINTLN(  globalSettings.hourToTurnLightOff);
  DEBUG_PRINTF(" | Hour to turn light ON: ");
  DEBUG_PRINTLN(  globalSettings.hourToTurnLightOn);
}
/*
   initSD() ...SD.begin() must be called before opening a file if the SD card has been removed and inserted...
*/
bool initSD() {
  if (!SD.begin(chipSelect)) {
    DEBUG_PRINTLNF("initialization failed. Things to check:");
    DEBUG_PRINTLNF("* is a card inserted?");
    DEBUG_PRINTLNF("* is your wiring correct?");
    DEBUG_PRINTLNF("* did you change the chipSelect pin to match your shield or module?");
    return false;
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
  if (!fInWarmUp) {
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
    writeSensorDataToLogFile();
    adjustCO2(sensorData.CO2Value);
  } else {
    DEBUG_PRINTLNF("In warm up - no readings taken.");
  }
}
/*
   doPump() turn the water pump on and set a callback when the pump should be turned off.
*/
void doPump() {
  if (!fInWarmUp) {
    writeEventHappened(PumpOn);
    digitalWrite(pumpPin, ON);
    Alarm.timerOnce((const unsigned long)globalSettings.amtSecsWaterPumpIsOn, turnPumpOff);
    DEBUG_PRINTF(" Pump ON for ");
    DEBUG_PRINT(globalSettings.amtSecsWaterPumpIsOn);
    DEBUG_PRINTLNF(" seconds.");
  }
}
/*
   turnPumpOff() - Turn the pump off after globalSettings.amtSecsWaterPumpIsOn.
*/
void turnPumpOff() {
  writeEventHappened(PumpOff);
  DEBUG_PRINTLNF("Turned pump OFF");
  digitalWrite(pumpPin, OFF);
}
/*
   turnCO2Off() - turn off the CO2 valve.
*/
void turnCO2Off() {
  writeEventHappened(CO2Off);
  DEBUG_PRINTLNF("Turned CO2 OFF");
  digitalWrite(CO2Pin, OFF);
}
void turnLightOnOrOff() {
  //light is off between 00:00:00 and 3:59:59
  if ( (hour() >= globalSettings.hourToTurnLightOff) && (hour() < globalSettings.hourToTurnLightOn) ) {
    turnLightOff();
  } else {
    turnLightOn();
  }
}
void turnLightOn() {
  writeEventHappened(LEDOn);
  fLEDon = true;
  DEBUG_PRINTLNF("turnLightOn fired");
  digitalWrite(LEDPin, ON);
}
void turnLightOff() {
  writeEventHappened(LEDOff);
  fLEDon = false;
  DEBUG_PRINTLNF("turnLightOff fired");
  digitalWrite(LEDPin, OFF);
}
/*
   warmUpOver() - The CO2 sensor is now available to take readings, so start taking readings on all the sensors.
*/
void warmUpOver() {
  fInWarmUp = false;
  writeEventHappened(WarmupEnd);
  turnLightOnOrOff();
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
/*
   adjustCO2() - if the LED is on, check to see if the CO2 is below 1200ppm but not -1.  If not, turn on the CO2 valve.
*/
void adjustCO2(int CO2Value) {
  if (fLEDon) {
    if (CO2Value < 0) { // The MH-Z16 returns a -1 when it couldn't get a valid reading.
      DEBUG_PRINTLNF("Could not get a valid CO2 reading - no adjustment made");
      return;
    }
    if (CO2Value <= 1200) { 
      //Got a good reading that is below 1200 ppm so turn on the CO2.  It is assume the valve is opened just
      //a little bit. Leave the valve on for less time as the value gets closer to 1200
      int nSecondsValveIsOpen = 0;
      CO2Value < 800 ? nSecondsValveIsOpen = 13 : nSecondsValveIsOpen = 8;
      writeEventHappened(CO2On);
      digitalWrite(CO2Pin, ON);
      Alarm.timerOnce((const unsigned long)nSecondsValveIsOpen, turnCO2Off);
    } 
  }else {
      DEBUG_PRINTLNF("The LED is off - no adjustment made");    
  }
}
/*
  writeSensorDataToLogFile() puts the date, time, and sensor readings into a String sensorString
  each property is separated by a common so the row can be read within a CSV file.
  I decided to use String instead of an array of char because the code has enough room to support it.
  Working with String is just a lot easier since I don't see coding as a strong skill of mine.
*/
void  writeSensorDataToLogFile() {
  //Just make string to value simpler by using String instead of an array of char..also give
  //enough room.
  String sensorString = String(50);
  File logFile = openFile();
  if (!logFile) {
    DEBUG_PRINTLNF("Could not write sensor data. Log File could NOT be opened!");
  } else {
    sensorString = String(SensorData) + ",";
    sensorString += getDateTimeString() + ",";
    sensorString += String(sensorData.temperatureValue) + ",";
    sensorString += String(sensorData.humidityValue) + ",";
    sensorString += sensorData.CO2Value;
    DEBUG_PRINTF("Sensor string: ");
    DEBUG_PRINTLN(sensorString);
    logFile.println(sensorString);
    logFile.flush();
    logFile.close();
  }
}
String getDateTimeString() {
  String dateTimeString = String(20);
  dateTimeString = String(month());
  dateTimeString += "/";
  dateTimeString += String(day());
  dateTimeString += "/";
  dateTimeString += String(year());
  dateTimeString += ",";
  dateTimeString += String(hour());
  dateTimeString += ":";
  dateTimeString += String(minute());
  dateTimeString += ":";
  dateTimeString += String(second());
  return dateTimeString;
}
String makeDateTimeString(time_t t) {
  tmElements_t tm;
  String dateTimeString = String(20);
  breakTime(t, tm);
  dateTimeString = String(tm.Month);
  dateTimeString += "/";
  dateTimeString += String(tm.Day);
  dateTimeString += "/";
  dateTimeString += String(tm.Year);
  dateTimeString += ",";
  dateTimeString += String(tm.Hour);
  dateTimeString += ":";
  dateTimeString += String(tm.Minute);
  dateTimeString += ":";
  dateTimeString += String(tm.Second);
  return dateTimeString;

}
File openFile() {
  currentCardState = (cardState_t)digitalRead(cardDetectPin);
  cardState_t cardState = currentCardState;
  cardState == prevCardState ? cardState = Unchanged : cardState = currentCardState;
  DEBUG_PRINTF("Previous state: ");
  DEBUG_PRINT(prevCardState);
  DEBUG_PRINTF(" | Current State: ");
  DEBUG_PRINT(currentCardState);
  DEBUG_PRINTF(" | Card State: ");
  DEBUG_PRINTLN(cardState);
  prevCardState = currentCardState;
  if (cardState == Inserted) {
    writeEventHappened(CardInserted);
    DEBUG_PRINTLNF("Card state is Inserted.");
    initSD();
  }
  //The file still needs to be opened if the cardState is Unchanged.  Writings will more likely
  //occur when the SD Card is inserted, which means the majority of the time cardState will be
  //UnChanged.  However, the file needs to be opened every time a row is logged.
  if (currentCardState == Inserted) {
    DEBUG_PRINTLNF("Opening file");
    return (SD.open(logFileName, FILE_WRITE));
  }
  //Couldn't open the file.
  return File();
}
/*
   closeSD() is the function called when the attachInterrupt(...) fires when the state of the Card
   Detect pin changes.  During test, there seems to be more changes of Card Detect state than I
   expected (e.g.: removing might generate three changes of state).  So I liberally call SD.end()
   when it is detected the card has been removed.
*/
void closeSD() {
  cardState_t cardState = (cardState_t)digitalRead(cardDetectPin);
  if (cardState == Removed) {
    writeEventHappened(CardRemoved);
    DEBUG_PRINTLNF("Card state is Removed");
    DEBUG_PRINTLNF("--> calling SD.end()");
    SD.end();
  } else {
    DEBUG_PRINTLNF("Card state is Inserted");
  }
}
/*
    writeEventHappened(...) log each of the events like turning the pump, LED, CO2 on or off.
*/
void writeEventHappened(logRow_t event) {
  if (event == CardInserted || event == CardRemoved) {
    return;
  }
  File logFile = openFile();
  if (!logFile) {
    DEBUG_PRINTF("Could not write event data for event: ");
    DEBUG_PRINT(event);
    DEBUG_PRINTLNF(" Log File could NOT be opened!");
  } else {
    String eventString = String(30);
    eventString = String(event) + ",";
    eventString += getDateTimeString();
    DEBUG_PRINTF("Event String: ");
    DEBUG_PRINTLN(eventString);
    logFile.println(eventString);
    logFile.flush();
    logFile.close();
  }
}
/*
   writeSettingsVersionToLogFile() -   Settings_V1 = 51
   51 will be the number used to identify the logfile contains sensor readings and actions
   based on globalSettingsV1_T variables
*/
void writeSettingsVersionToLogFile() {
  File logFile = openFile();
  if (!logFile) {
    DEBUG_PRINTLNF("Could not write settings data. Log File could NOT be opened!");
  } else {
    String settingsString = String(20);
    settingsString = String(Settings_V1) + ",";
    settingsString += getDateTimeString() ;
    DEBUG_PRINTF("Settings String: ");
    DEBUG_PRINTLN(settingsString);
    logFile.println(settingsString);
    logFile.flush();
    logFile.close();
  }
}



