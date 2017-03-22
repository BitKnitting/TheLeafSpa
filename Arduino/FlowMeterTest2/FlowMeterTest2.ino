#define DEBUG
#include <DebugLib.h>
#include <TimeAlarms.h>
#define FLOWSENSORPIN 2
const int secsBetweenTurningPumpON = 15 ;
const int amtSecsWaterPumpIsOn =  5 ;
volatile uint16_t pulses = 0;
void setup() {
  initStuff();
  DEBUG_PRINTLNF("***** FlowMeterTest2 *****");

}

void loop() {
  //The Alarm callbacks were not called unless Alarm.Delay() was in the loop.
  Alarm.delay(1);

}
void initStuff() {
  DEBUG_BEGIN;
  DEBUG_WAIT;
  pinMode(FLOWSENSORPIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(FLOWSENSORPIN), addPulse, RISING);
  doPump();
  // Tell the alarm to turn the pump on every secsBetweenTurningPumpOn seconds.
  Alarm.timerRepeat((const int)secsBetweenTurningPumpON, doPump);
}
void doPump() {
  //reset pulses so the pulse count occurs when the pump is supposed to be turned on for this run.
  pulses = 0;
  DEBUG_PRINTLNF("Turn pump on");
  Alarm.timerOnce((const unsigned int)amtSecsWaterPumpIsOn, turnPumpOff);
  DEBUG_PRINTF(" Pump ON for ");
  DEBUG_PRINT(amtSecsWaterPumpIsOn);
  DEBUG_PRINTLNF(" seconds.");
}
void turnPumpOff() {
  DEBUG_PRINTLNF("Turned pump OFF");
  float flowRate = pulses/7.5;
  DEBUG_PRINTF("Flow rate: ");
  DEBUG_PRINTLN(flowRate);
}
/*
 * addPulse() is a callback that happens when the (interrupt driven) digial pin assigned to the
 * flow meter detects a pulse.  A pulse occurs when the flow meter's pinwheel sensor makes a 
 * revolution - which happens when water is moving through it.
 */
void addPulse() {
  pulses++;
}

