/**************************************************************************
   RelayTest.ino is a very simple sketch to test using a relay.
   I wrote this as I was putting together the Leaf Spa prototype.
   Be Kind.  Margaret.
*/
// Set DEBUG to 0 if NOT connected to serial monitor (for debugging)
#define DEBUG 1
#include <DebugLib.h>
// Leave in the gunk for the sensors.  This way, we'll catch pin conflicts
// The Grove sensor is just a BoB using the DHT22
#include <DHT.h>
#define DHTPIN A0     // what pin we're connected to
// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
DHT dht(DHTPIN, DHTTYPE);
//1.Connect the Grove CO2 Sensor to Grove Base shield D7 Port
#include <SoftwareSerial.h>
SoftwareSerial CO2sensor(5, 6);     // TX, RX
// The GitHub location for the Timer library is here: //http://github.com/JChristensen/Timer
#include <Timer.h>
Timer t;
int   timeOnEvent;
//Identify relay pin
#define RELAYPIN4 7
/*
   SETUP
*/
void setup() {
  DEBUG_BEGIN;
  DEBUG_PRINTF("The amount of available ram: ");
  DEBUG_PRINTLN(freeRam());
  pinMode(RELAYPIN4, OUTPUT);
  t.every(1800000,turnPumpOn);   // 30 mins in ms => 1 min = 60 secs * 1000 ms/sec = 60,000 ms/min* 30mins  = 1,800,000 ms 
  //start by turning the pump on
  turnPumpOn();
}
/*
   LOOP
*/
void loop() {
  t.update();
}
/*
 * turnPumpOn - TBD on the amount of time...
 * HIGH turns on the Power Switch
 */
 void turnPumpOn() {
  digitalWrite(RELAYPIN4, HIGH);
  timeOnEvent = t.after(120000,turnPumpOff);       // 2 mins in ms => 2 * 60 secs/min * 1000 ms/sec = 
 }
 /*
  * turnPumpOff - after the amount of time the pump was on.
  * LOW turns off the Power Switch
  */
  void turnPumpOff() {
    digitalWrite(RELAYPIN4,LOW);
    t.stop(timeOnEvent);
  }

