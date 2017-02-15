// Set DEBUG to 0 if NOT connected to serial monitor (for debugging)
#define DEBUG 1
#include <DebugLib.h>
#include <TimeLib.h>
#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t
time_t timeCardWasRemoved;
#include <SPI.h>
#include <SD.h>
//     Adafruit SD shields and modules: pin 10
const int chipSelect = 10;
const int cardDetectPin = 3;
const int cardDetectInterrupPin = 3;
enum cardState_t {
  Inserted = LOW,
  Removed = HIGH,
  Unchanged = HIGH + 1
}prevCardState,currentCardState;
#define INSERTED LOW
#define REMOVED  HIGH




enum logRow_t {
  SensorData,
  Pump,
  LED,
  CO2,
  CardInserted,
  CardRemoved,
} logRowType;
const char *logFileName = "datalog.txt";
bool wasCardRemoved = false;
void setup() {
  // put your setup code here, to run once:
  initStuff();
}

void loop() {
  // put your main code here, to run repeatedly:
   currentCardState = (cardState_t)digitalRead(cardDetectPin);

   cardState_t cardState = currentCardState;
   cardState == prevCardState ? cardState = Unchanged : cardState=currentCardState;
      DEBUG_PRINTF("Previous state: ");
   DEBUG_PRINT(prevCardState);
   DEBUG_PRINTF(" | Current State: ");
   DEBUG_PRINT(currentCardState);
      DEBUG_PRINTF(" | Card State: ");
   DEBUG_PRINTLN(cardState);
   prevCardState = currentCardState;
  if (cardState == Inserted) {
    DEBUG_PRINTLNF("Card state is Inserted.");
    DEBUG_PRINTLNF("--> initializing SD Card");
    initSD();
  } else if (cardState == Removed) {
    DEBUG_PRINTLNF("Card state is Removed.");
    DEBUG_PRINTLNF("--> Calling SD.end()");
    SD.end();
  }else {
    DEBUG_PRINTLNF("Card state is Unchanged.");
  }
  delay(5000);

}
/*
   initStuff()
*/
void initStuff() {
  DEBUG_BEGIN;
  DEBUG_WAIT;
  DEBUG_PRINTF("The amount of available ram: ");
  DEBUG_PRINTLN(freeRam());
  setSyncProvider(RTC.get);   // the function to sync the time from the RTC..from Paul's example.
  if (timeStatus() != timeSet)
    DEBUG_PRINTLNF("Unable to sync with the RTC");
  else
    DEBUG_PRINTLNF("RTC has set the system time");
  pinMode(cardDetectPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(cardDetectPin), cardInsertOrRemoveDetected, CHANGE);
  //call in init to set whether the SD Card was in the reader when script started (or not).
  prevCardState = (cardState_t)digitalRead(cardDetectPin);
  DEBUG_PRINTF("Previous card state on init: ");
  DEBUG_PRINTLN(prevCardState);
}
void cardInsertOrRemoveDetected() {
  DEBUG_PRINTLNF("Detected");
}


/*
   initSD() ...SD.begin() must be called before opening a file if the SD card has been removed and inserted...
*/
bool initSD() {
  DEBUG_PRINTLNF("---> Right before SD.begin()");
  if (!SD.begin(chipSelect)) {
    DEBUG_PRINTLNF("initialization failed. Things to check:");
    DEBUG_PRINTLNF("* is a card inserted?");
    DEBUG_PRINTLNF("* is your wiring correct?");
    DEBUG_PRINTLNF("* did you change the chipSelect pin to match your shield or module?");
    return false;
  }
  return true;
}
