#define DEBUG 1
#include <DebugLib.h>
const int relayPin = 4; //arbitrarily use GPIO pin 4
#define CO2Sensor s_serial
const int defaultPin = 4;
int pin = defaultPin;
void setup() {
  DEBUG_BEGIN;
  DEBUG_PRINTF("The amount of available ram: ");
  DEBUG_PRINTLN(freeRam());
  pinMode(pin, OUTPUT);
  digitalWrite(pin,HIGH);
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
      case '\r':
      case '\n':
        break;
      case 'l': // set digital pin to LOW
      case 'L':
        {
          digitalWrite(pin, LOW);
          showMenu();
        }
        break;
      case 'h': // set digital pin to HIGH
      case 'H' :
        {
          digitalWrite(pin, HIGH);
          showMenu();
        }
        break;
      case 'p': // change which pin is being used
      case 'P' :
        {
          DEBUG_PRINTLNF("Enter switch pin number: ");
          while (Serial.available() == 0);
          bool lfnl = false;
          while (Serial.peek() == '\n' || Serial.peek() == '\r') {
            Serial.read(); 
            lfnl = true;
          }
          if (lfnl) {
            while (Serial.available() == 0);
          }
          pin = Serial.parseInt();
          pinMode(pin, OUTPUT);
          digitalWrite(pin,HIGH);
          DEBUG_PRINTF("Changed switch pin to pin ");
          DEBUG_PRINTLN(pin);
          showMenu();
        }
        break;
      case '?':// Display menu again.
        {
          showMenu();
        }
        break;
      default:
        break;
    }
  }
}
const char menuText[] PROGMEM =
  "\n"
  "Available commands:" "\n"
  "  ?     - shows available comands" "\n"
  "  p     - change pin number (default = 4)" "\n"
  "  h     - turn pin HIGH" "\n"
  "  l     - turn pin LOW \n"
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
