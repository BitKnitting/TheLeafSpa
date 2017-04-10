#include "CCS811.h"
#define HWID           0x20
#define i2cCCS811Addr  0x5B
const int wakePin = 7;
#include "errorMessages.h"
char errorBuffer[50] = {0};
CCS811Arduino ccs811(wakePin);
void setup() {
  Serial.begin(115200);
  Serial.println(F("********CCS811 Test********"));
  ccs811.init();
  byte hwID;
  ccs811.gethwID(&hwID);
  if (!ccs811.errCode) {
    Serial.print(F("Hardware ID: 0X"));
    Serial.println(hwID, HEX);
  } else {
    errorMessage(ccs811.errCode, errorBuffer);
    Serial.println(errorBuffer);
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}
