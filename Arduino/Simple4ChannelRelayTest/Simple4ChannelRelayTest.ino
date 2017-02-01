/*
 * Simple4ChannelRelayTest
 */
 #define DEBUG 1
#include <DebugLib.h>
 const int relayPin = 4; //arbitrarily use GPIO pin 4
void setup() {
  DEBUG_BEGIN;
  pinMode(relayPin, OUTPUT);

}

void loop() {
  //LOW turns on the relay
  digitalWrite(relayPin,HIGH);
  DEBUG_PRINTLNF("ON");
  delay(5000);
  digitalWrite(relayPin,LOW);
  DEBUG_PRINTLNF("OFF");
  delay(3000);

}
