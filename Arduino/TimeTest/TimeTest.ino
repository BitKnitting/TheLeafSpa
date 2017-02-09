// Set DEBUG to 0 if NOT connected to serial monitor (for debugging)
#define DEBUG 1
#include <DebugLib.h>
#include <TimeLib.h>
#include <Wire.h>
#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t
void setup() {
  DEBUG_BEGIN;
  DEBUG_PRINTF("The amount of available ram: ");
  DEBUG_PRINTLN(freeRam());
  setSyncProvider(RTC.get);   // the function to sync the time from the RTC
//    if(timeStatus()!= timeSet) 
//     DEBUG_PRINTLNF("Unable to sync with the RTC");
//  else
//     DEBUG_PRINTLNF("RTC has set the system time");      

  //Get the current time.
  time_t currentTime = now();
  time_t timeAtMidnight = tmConvert_t(year(),month(),day(),24,0,0);
  debugPrintDate(timeAtMidnight);
  time_t amtTimeUntilMidnight = timeAtMidnight - currentTime;
  DEBUG_PRINTF("Amount of time until midnight: ");
  showDuration(amtTimeUntilMidnight);
  DEBUG_PRINTLNF("");

}

void loop() {
  // put your main code here, to run repeatedly:

}
time_t tmConvert_t(int YYYY, byte MM, byte DD, byte hh, byte mm, byte ss)
{
  tmElements_t tmSet;
  tmSet.Year = YYYY - 1970;
  tmSet.Month = MM;
  tmSet.Day = DD;
  tmSet.Hour = hh;
  tmSet.Minute = mm;
  tmSet.Second = ss;
  return makeTime(tmSet);
}
//From Paul's Time library example TimeRTCLog
void showDuration(time_t duration) {
  // prints the duration in days, hours, minutes and seconds
  if (duration >= SECS_PER_DAY) {
    Serial.print(duration / SECS_PER_DAY);
    Serial.print(" day(s) ");
    duration = duration % SECS_PER_DAY;
  }
  if (duration >= SECS_PER_HOUR) {
    Serial.print(duration / SECS_PER_HOUR);
    Serial.print(" hour(s) ");
    duration = duration % SECS_PER_HOUR;
  }
  if (duration >= SECS_PER_MIN) {
    Serial.print(duration / SECS_PER_MIN);
    Serial.print(" minute(s) ");
    duration = duration % SECS_PER_MIN;
  }
  Serial.print(duration);
  Serial.print(" second(s) ");
}
void debugPrintDate(time_t t) {
  DEBUG_PRINTF("Date at midnight: ");
  DEBUG_PRINT(day(t));
  DEBUG_PRINTF("/");
  DEBUG_PRINT(month(t));
  DEBUG_PRINT("/");
  DEBUG_PRINTLN(year(t));
}
