const int cardDetectPin = 8;
void setup() {
  // put your setup code here, to run once:
Serial.begin(115200);
pinMode(cardDetectPin,INPUT);
}

void loop() {
  Serial.println(digitalRead(cardDetectPin) ? "HIGH" : "LOW");
  delay(1000);
  // put your main code here, to run repeatedly:

}
