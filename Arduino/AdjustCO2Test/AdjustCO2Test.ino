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
SoftwareSerial s_serial(5, 6);      // TX, RX
#define CO2Sensor s_serial
bool fContinuouslyRead = false;
// 1200ppm is the targeted CO2 level
const int setCO2Level = 1200;
const int CO2SwitchPin = 4;
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
  pinMode(CO2SwitchPin, OUTPUT);
  digitalWrite(CO2SwitchPin, HIGH);
  showMenu();
}
void loop() {
  serialHandler();
  if (fContinuouslyRead) {
    int co2Level = getCO2Reading(); 
    DEBUG_PRINTF("CO2 level: ");
    DEBUG_PRINTLN(co2Level);
  }
}
/*
   void serialHandler() handle user input from serial monitor (see showMenu()).
*/
void serialHandler() {
  char inChar;
  if ((inChar = Serial.read()) > 0) {
    switch (inChar) {
      case '\r':
      case '\n':
        break;
      case 'r': // take CO2 reading
      case 'R':
        {
          fContinuouslyRead = false;
          int co2Level = getCO2Reading();
          DEBUG_PRINTF("CO2 level: ");
          DEBUG_PRINTLN(co2Level);
          showMenu();
        }
        break;
      case 'c': // continuously read toggle
      case 'C' :
        {
          fContinuouslyRead = !fContinuouslyRead;
          if (!fContinuouslyRead) {
            showMenu();
          }
        }
        break;
      case 'o': // turn tank on for n seconds
      case 'O':
        {
          fContinuouslyRead = false;
          DEBUG_PRINTLNF("Enter number of secs to turn CO2 tank on: ");
          while (Serial.available() == 0);
          bool lfnl = false;
          while (Serial.peek() == '\n' || Serial.peek() == '\r') {
            Serial.read();
            lfnl = true;
          }
          if (lfnl) {
            while (Serial.available() == 0);
          }
          unsigned long nCO2TankSecs = Serial.parseInt();
          nCO2TankSecs < 1 ? 1 : nCO2TankSecs;
          DEBUG_PRINTF("Number of seconds to turn CO2 tank on: ");
          DEBUG_PRINTLN(nCO2TankSecs);
          turnTankOnFor(nCO2TankSecs);
        }
        break;
      case '?':
      case 'h': // Display menu again.
        {
          fContinuouslyRead = false;
          showMenu();
        }
        break;
      default:
        fContinuouslyRead = false;
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
  int co2Level = (int)data[2] * 256 + (int)data[3] - 100; //subtracting to adjust to Extech CO2 meter reading;
  return co2Level;
}
/*
   turnTankOn(int nSeconds) - turn the CO2 tank on for nSeconds.
*/
void turnTankOnFor(int nSeconds) {
  // Testing using the Power Switch....
  digitalWrite(CO2SwitchPin, LOW);
  delay(nSeconds * 1000);
  digitalWrite(CO2SwitchPin, HIGH);
  DEBUG_PRINTF("Turned CO2 on for ");
  DEBUG_PRINT(nSeconds);
  DEBUG_PRINTLNF(" seconds");
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
  "  c     - toggle continuously read CO2 on and off \n"
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
