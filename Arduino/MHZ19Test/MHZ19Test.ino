#define DEBUG
#include <DebugLib.h>
#define CO2Pin  10

void setup() {
  initStuff();
}

void loop() {
  //CO2 via pwm
  doCO2Test(1);
  doCO2Test(2);
  delay(1000);
}
void initStuff() {
  pinMode(CO2Pin, INPUT);     // declare sensor as input
  DEBUG_BEGIN;
  DEBUG_WAIT;
}
/*
   doCO2Test() assumes testNum = 1 or 2.
   see discussion here: http://electronics.stackexchange.com/questions/262473/mh-z19-co2-sensor-giving-diferent-values-using-uart-and-pwm
*/
void doCO2Test(uint8_t testNum) {
  unsigned long th, tl, ppm2, ppm3, tpwm, p1, p2 = 0;
  String whichTestStr = String(10);
  if (testNum == 1) {
    th = pulseIn(CO2Pin, HIGH, 1004000) / 1000;
    tl = 1004 - th;
    ppm2 = 2000 * (th - 2) / (th + tl - 4);
    ppm3 = 5000 * (th - 2) / (th + tl - 4);
    whichTestStr = "Test 1";
  } else {
    th = pulseIn(CO2Pin, HIGH, 3000000); // use microseconds
    tl = pulseIn(CO2Pin, LOW, 3000000);
    tpwm = th + tl; // actual pulse width
    ppm2 = 2000 * (th - p1) / (tpwm - p2);
    p1 = tpwm / 502; // start pulse width
    p2 = tpwm / 251; // start and end pulse width combined  th = pulseIn(pwmPin, HIGH, 3000000);
    ppm2 = 2000 * (th - p1) / (tpwm - p2);
    ppm3 = 5000 * (th - p1) / (tpwm - p2);
    whichTestStr = "Test 2";
  }
  DEBUG_PRINTLNF("---------------------------------------");
  DEBUG_PRINTLN(whichTestStr);
  DEBUG_PRINTF("th: ");
  DEBUG_PRINTLN(th);
  DEBUG_PRINTF("tl: ");
  DEBUG_PRINTLN(tl);
  DEBUG_PRINTF("2000ppm Concentration limit: ");
  DEBUG_PRINTLN(ppm2);
  DEBUG_PRINTF("5000ppm Concentration limit: ");
  DEBUG_PRINTLN(ppm3);
}

