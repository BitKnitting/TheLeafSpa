/*
   This is test code to get a better feel for the Time, TimeAlarms (and to a lesser but needed degree) DS1307RTC) and
   How to use the code to turn the LED light on and off.
   The GitHub locations for each library:
   Time - https://github.com/PaulStoffregen/Time
   TimeAlarms - https://github.com/PaulStoffregen/TimeAlarms
   DS1307RTC - https://github.com/PaulStoffregen/ds1307rtc
   I tried to document the code so that it is easy to follow.  But, since I also wrote the code this might be the case.

   The code plays around startLightsOffHour and stopLightsOfHour (using a 24 hour clock).  I set Arduino's time so the functions
   will trigger.

   Be Kind.  Margaret.
*/
// Set DEBUG to 0 if NOT connected to serial monitor (for debugging)
#define DEBUG 1
#include <DebugLib.h>
#include <TimeLib.h>
#include <TimeAlarms.h>
#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t
//The start and stop hours for when the light should be off.
const int startLightsOffHour = 0;
const int stopLightsOffHour = 4;
/*
   The relay turns the light on when the io pin is set to low.  I cover that here: https://bitknitting.wordpress.com/2017/02/03/build-log-for-february-2nd/
*/
#define ON LOW
#define OFF HIGH
#define LEDpin 7  //pin I'm using for the relay.
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
   initStuff() contains goo that is needed to set up the environment.
*/
void initStuff() {
  DEBUG_BEGIN;
  DEBUG_WAIT;
  DEBUG_PRINTF("The amount of available ram: ");
  DEBUG_PRINTLN(freeRam());
  //set the relay so LED is initially off.
  pinMode(LEDpin, OUTPUT);
  digitalWrite(LEDpin, OFF);
  setSyncProvider(RTC.get);   // the function to sync the time from the RTC..from Paul's example.
  if (timeStatus() != timeSet)
    DEBUG_PRINTLNF("Unable to sync with the RTC");
  else
    DEBUG_PRINTLNF("RTC has set the system time");
  //right now, the light needs to be on or off (most likely on).  
  turnLightOnOrOff();
  //set the time to right before an alarm is going to go off.  
  setCurrentTimeTo(0,0,0,-5);
  //set a timer to show the time every 10 seconds.  This gives a better feel for when the light on alarm will fire.
  Alarm.timerRepeat(10, showCurrentTime);
  //set the timers for turning the light on and off.
  Alarm.alarmRepeat(startLightsOffHour,0,0,turnLightOff);
  Alarm.alarmRepeat(stopLightsOffHour,0,0,turnLightOn);
}
/*
 * setCurrentTimeTo(...) this function is useful in testing to set the time right before an alarm would go off.  
 * timeSecs can be negative...i.e.: set timeSecs before or after.
 */
void setCurrentTimeTo(const int h,const int m, const int s,const long timeSecs) {
  tmElements_t tm;
  time_t currentTime = now();
  breakTime(currentTime, tm);
  tm.Hour = h;
  tm.Minute = m;
  tm.Second = s;
  time_t t= makeTime(tm) + timeSecs;
  setTime(t);
  showCurrentTime();
}
/*
   iturnLightOnOrOff() the light needs to be off or on - depending on what the current time is.
*/
void turnLightOnOrOff() {
  //light is off between 00:00:00 and 3:59:59
  if ( (hour() >= startLightsOffHour) && (hour() < stopLightsOffHour) ) {
    turnLightOff();
  } else {
    turnLightOn();
  }
}
void turnLightOn() {
  DEBUG_PRINTLNF("turnLightOn fired");
  digitalWrite(LEDpin, ON);
}
void turnLightOff() {
  DEBUG_PRINTLNF("turnLightOff fired");
  digitalWrite(LEDpin, OFF);
  setCurrentTimeTo(stopLightsOffHour,0,0,-5);
}
/*
   showCurrentTime() shows the current time..
*/
void showCurrentTime()
{
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.println();
}
/*
   Got this function from the example code in the libraries.
*/
void printDigits(int digits)
{
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

