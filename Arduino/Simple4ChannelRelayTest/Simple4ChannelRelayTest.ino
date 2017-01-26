/*
 * Simple4ChannelRelayTest
 */
 #define DEBUG 1
#include <DebugLib.h>
 const int relayPin = 4; //arbitrarily use GPIO pin 4
void setup() {
  DEBUG_BEGIN;
  DEBUG_PRINTF("The amount of available ram: ");
  DEBUG_PRINTLN(freeRam());
  pinMode(relayPin, OUTPUT);

}

void loop() {
  //LOW turns on the relay
  digitalWrite(relayPin,LOW);
  delay(3000);
  digitalWrite(relayPin,HIGH);
  delay(3000);

}
