
/*******************************************************************
   Arduino Firmware for The Leaf Spa.  I have been documenting this
   project on the bitknitting blog.  The intent is to control variables such
   as CO2 level, photoperiod, providing nutrients as well as monitor variables
   such as temperature, humidity, and CO2.

   If you find the code useful, it would be awesome if you could reference this work as
   you evolve yours.
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
   2 - Flow Meter
   3 - SD card detect
   4 - DHT temp/humidity
   5 - LED relay
   6 - CO2 relay
   7 - Software
   8 - Software Serial
   9 - Pump relay
   10 - SDI chip select
   11 - SDI DI pin
   12 - SDI DO pin
   13 - SDI CLK pin
 *******************************************************************/
#include <DHT.h>
#define DHTPIN 4
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
tmElements_t tm;
// RELAY pins
#define ON LOW //I discuss relay ON / OFF in this blog post: https://bitknitting.wordpress.com/2017/02/03/build-log-for-february-2nd/
#define OFF HIGH
#define pumpPin 9 //put the pin for the relay that will control the pump into this pin.
#define LEDPin  5 //same thing as for the pumpPin...
#define CO2Pin  6 //same things as for the other pins...
#define FlowMeterPin 2
volatile uint16_t pulses = 0;
//Use a flag to tell if the light is on.  Knowing if the light is on is important for adjusting CO2.
bool fLEDon = false;

//Use a flag to tell the code when the warm up period is over so that readings can be taken and logged.
bool fInWarmUp = true;
// EEPROM is used to load/save global settings.  See resetGlobalSettings() to get a feel for what properties are stored.
#define eepromWriteCheck 0x5678
#include <avr/eeprom.h>
struct globalSettingsV1_T
{
  unsigned int writeCheck;
  unsigned int       secsBtwnReadings;
  unsigned int       targetCO2Level;
  unsigned int       amtSecsWaterPumpIsOn;
  unsigned int       secsBetweenTurningPumpON;
  unsigned int       secsWarmUp;
  byte               hourToTurnLightOff;
  byte               hourToTurnLightOn;
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
  AmtSRAM,       //11
  // Initial Settings_V = 51
  //-> 52 added  FlowRate within PumpOff event.  Last checking of V51:
  // https://github.com/BitKnitting/TheLeafSpa/blob/8767e1d8ce863d0410f8f1d130abc79acc4fb78f/Arduino/TheLeafSpa/TheLeafSpa.ino
  // GitHub V = 8767e1d
  // -> 53 added RTCInit and setup events
  // Last check-in of V 52 -> https://github.com/BitKnitting/TheLeafSpa/blob/9eef66b64dd92caf9d721929ef88f9607e245f69/Arduino/TheLeafSpa/TheLeafSpa.ino
  // GitHub V = 873655b
  Settings_V = 53
} ;
const char *logFileName = "datalog.txt";
/*
   These two char arrays are used to hold logging info....I measured out the strings, and the full
   number of bytes is a bit less than 50 (around 45).  This is why stringBuffer is set to have 50 bytes.
   additionalInfo topped out at 18 bytes. This is why there are 20 bytes assigned to additionalInfo.
*/
char stringBuffer[60] = {0};
char additionalInfo[30];  // stringBuffer and additionalInfo are used for holding logging info.

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
  setSyncProvider(RTC.get);
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
  pinMode(pumpPin, OUTPUT);
  pinMode(LEDPin, OUTPUT);
  pinMode(CO2Pin, OUTPUT);
  digitalWrite(pumpPin, OFF);
  digitalWrite(LEDPin, OFF);
  digitalWrite(CO2Pin, OFF);
  pinMode(FlowMeterPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(FlowMeterPin), addPulse, RISING);
  loadGlobalSettings();
  // Set up reading the sensors.
  Alarm.timerRepeat((const int)globalSettings.secsBtwnReadings, doReading); //set the timer up to go off when it is time to take sensor readings.
  // Set up turning the pump off and on.
  Alarm.timerRepeat((const int)globalSettings.secsBetweenTurningPumpON, doPump);
  // Set up LED photoperiod through on and off alarms that fire every day.
  Alarm.alarmRepeat((const int)globalSettings.hourToTurnLightOff, 0, 0, turnLightOff);
  Alarm.alarmRepeat((const int)globalSettings.hourToTurnLightOn, 0, 0, turnLightOn);
  // Set a timer for the number of minutes needed to warm up the CO2 sensor before taking readings
  fInWarmUp = true;
  writeEventHappened(WarmupStart, "");
  Alarm.timerOnce((const int)globalSettings.secsWarmUp, warmUpOver);
}
/*
   loadGlobalSettings() uses EEPROM to get the settings variables identified within the globalSettings structure.
   if the write check determines the settings are either not there or the write check itself is corrupt, the settings
   are reset to default values.'
*/
void loadGlobalSettings() {
  eeprom_read_block(&globalSettings, (void *)0, sizeof(globalSettings));
  //if its a first time setup or our magic number in eeprom is wrong reset to default
  //NOTE: Not doing this check.  Always use what it is resetGlobalSettings() since at this point there is no user interaction...
  //if (globalSettings.writeCheck != eepromWriteCheck) {
  resetGlobalSettings();
  // }
  // Write the settings to the log file.  This way we'll know when lights, pump, CO2 should turn
  // on and off, etc.
  writeSettings();
}
/*
   resetGlobalSettings() variables to default values.  See LoadGlobalSettings() to get an idea when this function is called.
*/
void resetGlobalSettings() {
  globalSettings.writeCheck = eepromWriteCheck;
  globalSettings.secsBtwnReadings = 60  ;
  globalSettings.targetCO2Level = 1200;
  globalSettings.secsWarmUp = 180 ; //The Grove wiki states CO2 sensor should warm up for 3 minutes:https://seeeddoc.github.io/Grove-CO2_Sensor/
  globalSettings.amtSecsWaterPumpIsOn = 60; //amount of seconds for pump to be ON.
  globalSettings.secsBetweenTurningPumpON = 30 * 60; //# secs between turning pump ON.
  globalSettings.hourToTurnLightOff = 0;
  globalSettings.hourToTurnLightOn = 8;
  eeprom_write_block(&globalSettings, (void *)0, sizeof(globalSettings)); //write settings to eeprom
}
/*
   initSD() ...SD.begin() must be called before opening a file if the SD card has been removed and inserted...
*/
bool initSD() {
  if (!SD.begin(chipSelect)) {
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
  if (!fInWarmUp) {
    sensorData.temperatureValue = dht.readTemperature();
    sensorData.humidityValue = dht.readHumidity();
    sensorData.CO2Value = takeCO2Reading();
    writeSensorDataToLogFile();
    adjustCO2(sensorData.CO2Value);
  }
}
/*
   doPump() turn the water pump on and set a callback when the pump should be turned off.
*/
void doPump() {
  if (!fInWarmUp) {
    pulses = 0;
    writeEventHappened(PumpOn, "");
    digitalWrite(pumpPin, ON);
    Alarm.timerOnce((const unsigned long)globalSettings.amtSecsWaterPumpIsOn, turnPumpOff);
  }
}
/*
   turnPumpOff() - Turn the pump off after globalSettings.amtSecsWaterPumpIsOn.
*/
void turnPumpOff() {
  float flowRate = pulses / 7.5;
  char flowRateStr[6];
  dtostrf(flowRate, 5, 1, flowRateStr);
  writeEventHappened(PumpOff, flowRateStr);
  digitalWrite(pumpPin, OFF);
}
/*
   turnCO2Off() - turn off the CO2 valve.
*/
void turnCO2Off() {
  writeEventHappened(CO2Off, "");
  digitalWrite(CO2Pin, OFF);
}
void turnLightOnOrOff() {
  if ( (hour() >= globalSettings.hourToTurnLightOff) && (hour() < globalSettings.hourToTurnLightOn) ) {
    turnLightOff();
  } else {
    turnLightOn();
  }
}
void turnLightOn() {
  writeEventHappened(LEDOn, "");
  fLEDon = true;
  digitalWrite(LEDPin, ON);
}
void turnLightOff() {
  writeEventHappened(LEDOff, "");
  fLEDon = false;
  digitalWrite(LEDPin, OFF);
}
/*
   warmUpOver() - The CO2 sensor is now available to take readings, so start taking readings on all the sensors.
*/
void warmUpOver() {
  fInWarmUp = false;
  writeEventHappened(WarmupEnd, "");
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
      return;
    }
    if (CO2Value <= 1200) {
      //Got a good reading that is below 1200 ppm so turn on the CO2.  It is assume the valve is opened just
      //a little bit. Leave the valve on for less time as the value gets closer to 1200
      int nSecondsValveIsOpen = 0;
      CO2Value < 800 ? nSecondsValveIsOpen = 8 : nSecondsValveIsOpen = 5;
      writeEventHappened(CO2On, "");
      digitalWrite(CO2Pin, ON);
      Alarm.timerOnce((const unsigned long)nSecondsValveIsOpen, turnCO2Off);
    }
  }
}
/*
   openFile() -> open the log file for writing out events...deals with the SD card being inserted/removed...something I wanted
   to support so the card can be removed while the firmware is running and then reinserted after the contents of the card are transfered
   to my mac.  Note: In the future I'd rather do this over some form of wireless.
*/
File openFile() {
  currentCardState = (cardState_t)digitalRead(cardDetectPin);
  cardState_t cardState = currentCardState;
  cardState == prevCardState ? cardState = Unchanged : cardState = currentCardState;
  prevCardState = currentCardState;
  if (cardState == Inserted) {
    writeEventHappened(CardInserted, "");
    initSD();
  }
  //The file still needs to be opened if the cardState is Unchanged.  Writings will more likely
  //occur when the SD Card is inserted, which means the majority of the time cardState will be
  //UnChanged.  However, the file needs to be opened every time a row is logged.
  if (currentCardState == Inserted) {
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
    SD.end();
  }
}

/*
   addPulse() is a callback that happens when the (interrupt driven) digial pin assigned to the
   flow meter detects a pulse.  A pulse occurs when the flow meter's pinwheel sensor makes a
   revolution - which happens when water is moving through it.
*/
void addPulse() {
  pulses++;
}

/*
   Write a record to the log file that contains the current Leaf Spa settings
*/
void writeSettings() {
  itoa(globalSettings.secsBtwnReadings, additionalInfo, 10);
  char *idx = additionalInfo + strlen(additionalInfo);
  *idx++ = ',';
  itoa(globalSettings.targetCO2Level, idx, 10);
  idx = additionalInfo + strlen(additionalInfo);
  *idx++ = ',';
  itoa(globalSettings.secsWarmUp, idx, 10);
  idx = additionalInfo + strlen(additionalInfo);
  *idx++ = ',';
  itoa(globalSettings.amtSecsWaterPumpIsOn, idx, 10);
  idx = additionalInfo + strlen(additionalInfo);
  *idx++ = ',';
  itoa(globalSettings.secsBetweenTurningPumpON, idx, 10);
  idx = additionalInfo + strlen(additionalInfo);
  *idx++ = ',';
  itoa(globalSettings.hourToTurnLightOff, idx, 10);
  idx = additionalInfo + strlen(additionalInfo);
  *idx++ = ',';
  itoa(globalSettings.hourToTurnLightOn, idx, 10);
  writeEventHappened(Settings_V, additionalInfo);
}
/*
  writeSensorDataToLogFile() puts the date, time, and sensor readings into a String sensorString
  each property is separated by a common so the row can be read within a CSV file.
  I decided to use String instead of an array of char because the code has enough room to support it.
  Working with String is just a lot easier since I don't see coding as a strong skill of mine.
*/
void  writeSensorDataToLogFile() {
  //  File logFile = openFile();
  //  if (!logFile) {
  //    return;
  //  }
  dtostrf(sensorData.temperatureValue, 4, 1, additionalInfo);
  char *idx = additionalInfo + strlen(additionalInfo);
  *idx++ = ',';
  dtostrf(sensorData.humidityValue, 5, 1, idx);
  idx = additionalInfo + strlen(additionalInfo);
  *idx++ = ',';
  itoa(sensorData.CO2Value, idx, 10);
  writeEventHappened(SensorData, additionalInfo);
}
/*
    writeEventHappened(...) log each of the events like turning the pump, LED, CO2 on or off.
    BE CAREFUL NOT TO OVERFILL THE String BUFFER.
*/
void writeEventHappened(logRow_t event, char * additionalInfo) {
  if (event == CardInserted || event == CardRemoved) {
    return;
  }
  File logFile = openFile();
  if (!logFile) {
    return;
  }
  //  writeFreeRamtoStringBuffer();
  //  logFile.println(stringBuffer);
  ////////////////////////////////////////////////////////////////////////////
  //
  // an event has a log row type followed by date/time followed by additonal info.
  itoa(event, stringBuffer, 10);
  char *idx = stringBuffer + strlen(stringBuffer);
  *idx++ = ',';
  makeDateTimeString(idx);
  idx = stringBuffer + strlen(stringBuffer);
  *idx++ = ',';
  strcpy(idx, additionalInfo);
  logFile.println(stringBuffer);
  //  writeFreeRamtoStringBuffer();
  //  logFile.println(stringBuffer);
  ////////////////////////////////////////////////////////////////////////////
  logFile.flush();
  logFile.close();
}
void makeDateTimeString(char *dateAndTime) {
  itoa(month(), dateAndTime, 10);
  char *idx = dateAndTime + strlen(dateAndTime);
  *idx++ = '/';
  itoa(day(), idx, 10);
  idx = dateAndTime + strlen(dateAndTime);
  *idx++ = '/';
  itoa(year(), idx, 10);
  idx = dateAndTime + strlen(dateAndTime);
  *idx++ = ',';
  itoa(hour(), idx, 10);
  idx = dateAndTime + strlen(dateAndTime);
  *idx++ = ':';
  itoa(minute(), idx, 10);
  idx = dateAndTime + strlen(dateAndTime);
  *idx++ = ':';
  itoa(second(), idx, 10);
}
/*
  Write the amount of SRAM available after the record has been written.  If there is a change, it's of interest
  to know the length of stringBuffer and additionalInfo.  Are they what I expect them to be?
*/
void writeFreeRamtoStringBuffer() {
  int sram = freeRam();
  itoa(AmtSRAM, stringBuffer, 10);
  char *idx = stringBuffer + strlen(stringBuffer);
  *idx++ = ',';
  itoa(sram, idx, 10);
  idx = stringBuffer + strlen(stringBuffer);
  *idx++ = ',';
  itoa(strlen(stringBuffer), idx, 10);
  idx = stringBuffer + strlen(stringBuffer);
  *idx++ = ',';
  itoa(strlen(additionalInfo), idx, 10);
}
/*
   free_ram()
*/
int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int)&v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
