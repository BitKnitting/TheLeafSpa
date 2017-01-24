/*
   Part of The Leaf Spa Testing
   Be Kind. Margaret Johnson.
*/
// Set DEBUG to 0 if NOT connected to serial monitor (for debugging)
#define DEBUG 1
#include <DebugLib.h>
/*
   Use the Timer library to take periodic readings. See: //http://github.com/JChristensen/Timer
*/
#include <Timer.h>
Timer t;
int   timeOnEvent;
/*
   Using Adafruit's SD Card Shield
*/
// include the SD library:
#include <SPI.h>
#include <SD.h>

// Adafruit SD shields and modules: pin 10
const int chipSelect = 10;
/*
   Using the Grove Base Shield and Grove CO2 monitor plugged into the shield.
*/
#include <SoftwareSerial.h>
SoftwareSerial s_serial(7, 8);      // TX, RX
#define CO2Sensor s_serial

const unsigned char cmd_get_sensor[] =
{
  0xff, 0x01, 0x86, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x79
};

void setup() {
  DEBUG_BEGIN;
  DEBUG_PRINTF("The amount of available ram: ");
  DEBUG_PRINTLN(freeRam());
  DEBUG_PRINTF("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    DEBUG_PRINTLNF("initialization failed. Things to check:");
    DEBUG_PRINTLNF("* is a card inserted?");
    DEBUG_PRINTLNF("* is your wiring correct?");
    DEBUG_PRINTLNF("* did you change the chipSelect pin to match your shield or module?");
    blink(30000);
    return;   //That's it...have to log data or running is pointless...
  } else {
    DEBUG_PRINTLNF("SD Card initialized.");
    CO2Sensor.begin(9600);
  }
  t.every(60000, takeCO2Reading); //take CO2 measurements every 1 minute -> 1 mins * 60 sec/min * 1000 ms/sec = 60,000
  takeCO2Reading();
}
void loop() {
  t.update();
}
/* takeCO2Reading() wrapper around getCO2Reading() to check if a reading happened..writes to log file

*/
void takeCO2Reading() {
  int CO2Reading = getCO2Reading();
  DEBUG_PRINTF("CO2 reading: ");
  DEBUG_PRINTLN(CO2Reading);
  if (CO2Reading < 0) {
    blink(5);
  } else {
    File dataFile = SD.open("datalog.txt", FILE_WRITE);
    if (dataFile) {
      dataFile.println(CO2Reading);
      dataFile.close();
    } else {
      DEBUG_PRINTLNF("ERROR! Could not open data file...");
      blink(5);
    }
  }
}
/*
   takeCO2Reading() takes a CO2 reading then logs onto the SD Card
*/
int getCO2Reading() {
  byte data[9];
  int i = 0;
  //transmit get CO2 reading command
  for (i = 0; i < sizeof(cmd_get_sensor); i++)
  {
    CO2Sensor.write(cmd_get_sensor[i]);
  }
  delay(10);
  //get reading
  if (CO2Sensor.available())
  {
    while (CO2Sensor.available())
    {
      for (int i = 0; i < 9; i++)
      {
        data[i] = CO2Sensor.read();
      }
    }
  }
  if ((i != 9) || (1 + (0xFF ^ (byte)(data[1] + data[2] + data[3] + data[4] + data[5] + data[6] + data[7]))) != data[8])
  {
    return -1;
  }

  return ( (int)data[2] * 256 + (int)data[3]);
}
/*
    blink() called when ran into an error that stopped code from running correctly
*/
void blink(int timesToLoop) {
  for (int i = 0; i < timesToLoop; i++) {
    digitalWrite(13, HIGH);
    delay(1000);
    digitalWrite(13, LOW);
    delay(1000);
  }
}
