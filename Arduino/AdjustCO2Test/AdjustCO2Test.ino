/*
  Part of The Leaf Spa Work.
  ..how long should CO2 be released into the Grow Chamber in order to reach the target set CO2 level ?
  Be Kind. Margaret Johnson.
*/
// For this test DEBUG must be 1.  The UI relies on serial interfce....
#define DEBUG 1
#include <DebugLib.h>
/*
   Using the Grove Base Shield and Grove CO2 monitor plugged into the shield.
*/
#include <SoftwareSerial.h>
SoftwareSerial s_serial(7, 8);      // TX, RX
#define CO2Sensor s_serial
// 1200ppm is the targeted CO2 level
const int setCO2Level = 1200;

const unsigned char cmd_get_sensor[] =
{
  0xff, 0x01, 0x86, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x79
};

void setup() {
  DEBUG_BEGIN;
  DEBUG_PRINTF("The amount of available ram: ");
  DEBUG_PRINTLN(freeRam());
  CO2Sensor.begin(9600);
  showMenu();
}
void loop() {
  serialHandler();
}
/*
   void serialHandler() handle user input from serial monitor (see showMenu()).
*/
void serialHandler() {
  char inChar;
  if ((inChar = Serial.read()) > 0) {
    switch (inChar) {
      case 'r': // take CO2 reading
      case 'R':
        {
          int co2Level = getCO2Reading();
          DEBUG_PRINTF("CO2 level: ");
          DEBUG_PRINTLN(co2Level);
          showMenu();
        }
        break;
      case 'o': // turn tank on for n seconds
      case 'O':
        {
          DEBUG_PRINTLNF("Enter number of secs to turn CO2 tank on: ");
          while (Serial.available() == 0);
          int nCO2TankSecs = Serial.parseInt();
          nCO2TankSecs < 1 ? 1 : nCO2TankSecs;
          DEBUG_PRINTF("Number of seconds to turn CO2 tank on: ");
          DEBUG_PRINTLN(nCO2TankSecs);
          turnTankOnFor(nCO2TankSecs);
        }
        break;
      case '?':
      case 'h': // Display menu again.
        {
          showMenu();
        }
        break;
      default:
        break;
    }
  }
}


/*
   getCO2Reading() gets a CO2 reading.
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
  int co2Level = (int)data[2] * 256 + (int)data[3] - 60; //subtracting 60ppm to adjust to Extech CO2 meter reading;
  return co2Level;
}
/*
   turnTankOn(int nSeconds) - turn the CO2 tank on for nSeconds.
*/
const int CO2SwitchPin = 4; //arbitrarily using io pin 4...
void turnTankOnFor(int nSeconds) {
  // Testing using the Power Switch....
  digitalWrite(CO2SwitchPin, HIGH);
  delay(nSeconds);
  digitalWrite(CO2SwitchPin, LOW);
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
const char menuText[] PROGMEM =
  "\n"
  "Available commands:" "\n"
  "  ?     - shows available comands" "\n"
  "  r     - take CO2 reading" "\n"
  "  o     - turn CO2 tank on for n seconds" "\n"
  ;
/*-----------------------------------------------------------
  show command line menu
  -----------------------------------------------------------*/
static void showMenu () {
  showString(menuText);
}
static void showString (PGM_P s) {
  for (;;) {
    char c = pgm_read_byte(s++);
    if (c == 0)
      break;
    if (c == '\n')
      Serial.print('\r');
    Serial.print(c);
  }
}
