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
//Identify relay pin
#define RELAYPIN1 4 //D4 on the Grove Shield
/*
   SETUP
*/
void setup() {
  DEBUG_BEGIN;
  DEBUG_PRINTF("The amount of available ram: ");
  DEBUG_PRINTLN(freeRam());
  pinMode(RELAYPIN1, OUTPUT);
}
/*
   LOOP
*/
void loop() {
  //Turn relay ON
  digitalWrite(RELAYPIN1, LOW);
  //Wait 10 seconds
  delay(10000);
  //Turn relay OFF
  digitalWrite(RELAYPIN1, HIGH);
  //Wait 10 seconds
  delay(10000);
}
