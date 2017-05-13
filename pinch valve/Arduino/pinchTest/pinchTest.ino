/*
This test script is used to run the servo motor being used as the motor part of a pinch valve.
The pinch valve pinches on a hose to prevent liquid coming out.  The hose is the type used in
aquariums, but soft (as possible)
*/

#include <Servo.h> 

int servoPin = 9;
 
Servo servo;  
 
int angle = 0;   // servo position in degrees 
 
void setup() 
{ 
  Serial.begin(115200);
  servo.attach(servoPin);
  servo.write(0);
  showMenu();
} 
 
 
void loop() 
{ 
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
      case 'c': // close valve
      case 'C':
        {
          servo.write(0);
          showMenu();
        }
        break;
      case 'o': // open valve
      case 'O' :
        {
          servo.write(90);
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
  "  c     - close valve" "\n"
  "  o     - open valve" "\n"
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
