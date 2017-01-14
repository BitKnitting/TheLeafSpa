/*
  Grow OS ver. 0.991 beta
  Casabonita420 15/06/2016
  *USE tweaked twi.c file provided, check README.TXT*
*/

// Include our libraries
#include <LiquidCrystal.h>
#include <DHT.h>
#include <Encoder.h>
#include <EEPROM.h>
#include <string.h>
#include <NDIRZ16.h>
#include <SoftwareSerial.h>

#define blPin             7         // Backlight pin
#define rotaryBtPin       4         // Rotary button pin. Second pin to Ground
#define rotaryAPin        2         // Rotary A pin
#define rotaryBPin        3         // Rotary B pin
#define lightPin          A0        // Light photoresistor pin
#define DHTPIN            6         // DHT22 temperature-humidity pin
#define relayPin1         5         // High Low Exhaust relay pin
#define relayPin2         A1        // On - Off exhaust relay pin
#define relayPin3         A3        // Co2 relay pin
#define relayPin4         A2        // Dehumidifier relay pin
#define sensorRefTime     2000      // Sensors refresh interval (millis)
#define longPressTime     1200      // Rotary button long press interval
#define rotaryRefTime     5         // Rotary refresh interval
#define screenRefTime     100       // Screen refresh interval
#define computeRefTime    500       // Compute relays interval
#define blTimeout         60000     // Backlight timeout interval
#define maxDehumAddr      30        // Max Dehumidier EEPROM address
#define deltaDehumAddr    31        // Delta Dehumidier EEPROM address
#define maxTempAddr       32        // Max Temperature EEPROM address
#define deltaTempAddr     33        // Delta Temperature EEPROM address
#define maxHumAddr        34        // Max Humidity EEPROM address
#define deltaHumAddr      35        // Delta Humidity EEPROM address
#define maxPpmAddr        36        // Max CO2 ppm EEPROM address
#define deltaPpmAddr      37        // Delta CO2 ppm EEPROM address      
#define loadCheckAddr     38        // First run check EEPROM address
#define loadCheck         128       // First run check value
#define lightTh           100       // Photoresistor threshold value (light on/lights off)
#define rotaryMultiplier  4         // !ATTENTION! Some rotary encoders increase/decrease by 1, and others by 4. Change this value according to your encoder

typedef enum GROW_STATE             // Create Grow OS states
{
  GROW_STATE_IDLE,
  GROW_STATE_PROGRAMMING_MAX_TEMP,
  GROW_STATE_PROGRAMMING_DELTA_TEMP,
  GROW_STATE_PROGRAMMING_MAX_HUM,
  GROW_STATE_PROGRAMMING_DELTA_HUM,
  GROW_STATE_PROGRAMMING_MAX_DEHUM,
  GROW_STATE_PROGRAMMING_DELTA_DEHUM,
  GROW_STATE_PROGRAMMING_MAX_PPM,
  GROW_STATE_PROGRAMMING_DELTA_PPM,
  GROW_STATE_PANIC
}
growState_t;
growState_t growState;

float hum = 0, temp = 0;
bool htOK = true, rotaryBtPressed = false, rotaryBtLPressed = false, rotaryBtRead = false, wakeUp = true;
bool rotaryBtLRead = false, encoderRead, seeLight = false, exhaustOn = false, exhaustHigh = false;
bool co2ValveOn = false, dehumOn = false, blinkText = false, co2Error = false, co2SensorOn = false;
unsigned long sensorsTS =  0, rotaryBtUpTS = 0, rotaryTurnTS = 0, lightTS = 0, screenRefTS = 0, blTimeoutTS = 0, computeRefTS = 0;    // TimeStamps
unsigned long co2Level = 0;
long posOldRotary = 0, posNewRotary = 0;
uint16_t lightLevel = 0, maxTemp = 0, deltaTemp = 0, maxHum = 0, deltaHum = 0, maxDehum = 0, deltaDehum = 0, maxPpm = 0, deltaPpm = 0, panicCounter = 0;
uint8_t resetCounter = 0, blinkCounter = 0, panicCode = 0;

dht DHT;                                            // Create DHT22 sensor object

Encoder rotaryEnc(rotaryAPin, rotaryBPin);          // Create rotary object (rotary pin A, rotary pin B), middle pin ground

// Initialize the LCD library with the numbers of the interface pins
LiquidCrystal lcd(13, 12, 11, 10, 9, 8);            // (RS, EN, D4, D5, D6, D7)

//Arduino Pin D21 (Software Serial Rx) <===> Adaptor's Green  Wire (Tx)
//Arduino Pin D20 (Software Serial Tx) <===> Adaptor's Yellow Wire (Rx)
SoftwareSerial mySerial(51, 50);
NDIRZ16 mySensor = NDIRZ16(&mySerial);

//--------->-------------------------->-------------------------->----------------------------SETUP----------------------------->
void setup() {
  mySerial.begin(9600);
  pinMode(blPin, OUTPUT);
  pinMode(rotaryBtPin, INPUT_PULLUP);
  pinMode(lightPin, INPUT);
  pinMode(relayPin1, OUTPUT);
  pinMode(relayPin2, OUTPUT);
  pinMode(relayPin3, OUTPUT);
  pinMode(relayPin4, OUTPUT);
  digitalWrite(blPin, HIGH);                    // Set up backlight
  digitalWrite(relayPin1, HIGH);                // Reversed relays!!!!!!!!!!!!!
  digitalWrite(relayPin2, HIGH);
  digitalWrite(relayPin3, HIGH);
  digitalWrite(relayPin4, HIGH);
  lcd.begin(16, 2);                             // Set up the LCD's number of columns and rows:
  initialize();
  // Debugging - check resets
  if (EEPROM.read(40) == 128) {
    resetCounter = EEPROM.read(39) + 1;
    if (resetCounter > 99) resetCounter = 1;
    EEPROM.write(39, resetCounter);
  } else {
    resetCounter = 1;
    EEPROM.write(39, resetCounter);
    EEPROM.write(40, 128);
  }
}
//---------<--------------------------<--------------------------<----------------------------<-----------------------------<

//--------->-------------------------->-------------------------->----------------------------LOOP----------------------------->
void loop() {

  if ((unsigned long)millis() - sensorsTS > sensorRefTime ) {                // Read sensors every "sensorRefTime"  millis
    sensorsTS = millis();
    readSensors();
  }

  readButtonPr();                                                            // Read button input as fast as possible

  if ((unsigned long)millis() - rotaryTurnTS > rotaryRefTime ) {             // Read rotary input every "rotaryRefTime" millis
    rotaryTurnTS = millis();
    readRotary();
  }

  if ((unsigned long)millis() - screenRefTS > screenRefTime ) {              // Refresh screen every "screenRefTime"  millis
    screenRefTS = millis();
    screenRef();
    blinkCounter++;                                                          // Blink progamming mode values every 400 millis
    if (blinkCounter > 400 / screenRefTime) {
      blinkText = !blinkText;
      blinkCounter = 0;
    }
  }

  if ((unsigned long)millis() - blTimeoutTS > blTimeout  && wakeUp) {        // Backlight timeout after "blTimeout" millis
    digitalWrite(blPin, LOW);
    wakeUp = false;
  }


  if ((unsigned long)millis() - computeRefTS  >  computeRefTime) {        // Compute relays' output
    computeRefTS = millis();
    if (!htOK || (co2Error && co2SensorOn)) {
      panicCounter++;
      if (!htOK) {
        if (panicCounter > 30000 / computeRefTime) {
          panicCounter = 0;
          panicCode = 1;
          panic();
        }
      }
      if (co2Error && co2SensorOn) {
        if (panicCounter > 30000 / computeRefTime) {
          panicCounter = 0;
          panicCode = 3;
          panic();
        }
      }
    } else {
      if (panicCounter > 0) {
        panicCounter = 0;
      }
    }
    if (growState != GROW_STATE_PANIC) compute();
  }

}
//---------<--------------------------<--------------------------<----------------------------<-----------------------------<

//--------->-------------------------->-------------------------->----------------------------READ SENSORS----------------------------->
void readSensors() {
  int chk = DHT.read22(DHTPIN);                         // Check DHT22 connection
  if (chk == DHTLIB_OK) {                               // If DHT22 connection is OK, read values
    hum = DHT.humidity;                                 // Read humidity
    temp = DHT.temperature;                             // Read temperature
    htOK = true;
  } else {
    htOK = false;
  }
  lightLevel = analogRead(lightPin);                    // Read voltage level of photoresistor voltage divider. 5V----Photoresistor----Arduino----10Kohm----Ground
  if (lightLevel > lightTh) seeLight = true;            // 0-5V -> 0-1024 value. If photoresistor voltage is greater than our threshold, we see light
  else seeLight = false;
  if (co2SensorOn) {
    if (mySensor.measure()) {
      co2Level = mySensor.ppm;
      if (co2Level > 0 & co2Level < 10000) co2Error = false;
      else co2Error = true;
    } else {
      co2Error = true;
    }
  }
}
//---------<--------------------------<--------------------------<----------------------------<-----------------------------<

//--------->-------------------------->-------------------------->----------------------------READ BUTTON PRESS----------------------------->
void readButtonPr() {
  rotaryBtPressed = !digitalRead(rotaryBtPin);
  if (rotaryBtPressed) {
    if (!rotaryBtRead) {
      rotaryBtUpTS = millis();
      rotaryBtRead = true;
    } else {
      if (millis() - rotaryBtUpTS > longPressTime) {       // If you hold the button LONGER than "longPressTime", it's a longPress
        if (!rotaryBtLRead) {
          rotaryBtLPr();
          rotaryBtLRead = true;
        }
      }
    }
  } else {
    if (rotaryBtRead) {
      if (millis() - rotaryBtUpTS <= longPressTime) {
        rotaryBtPr();                                     // If you hold the button LESS than "longPressTime", it's a simplePress
      }
    }
    rotaryBtRead = false;
    rotaryBtLRead = false;
  }
}
//---------<--------------------------<--------------------------<----------------------------<-----------------------------<

//--------->-------------------------->-------------------------->----------------------------BUTTON SIMPLE PRESS FUNCTION----------------------------->
void rotaryBtPr() {
  wakeUpFunc();                                                     // Enable backlight
  if (growState != GROW_STATE_IDLE) lcd.clear();
  switch (growState) {
    case GROW_STATE_PROGRAMMING_MAX_TEMP :
      rotaryEnc.write(deltaTemp * rotaryMultiplier);                // Set rotary position according to our next value
      growState = GROW_STATE_PROGRAMMING_DELTA_TEMP;                // Set programming mode value and go to the next one
      break;
    case GROW_STATE_PROGRAMMING_DELTA_TEMP :
      rotaryEnc.write(maxHum * rotaryMultiplier);
      growState = GROW_STATE_PROGRAMMING_MAX_HUM;
      break;
    case GROW_STATE_PROGRAMMING_MAX_HUM :
      rotaryEnc.write(deltaHum * rotaryMultiplier);
      growState = GROW_STATE_PROGRAMMING_DELTA_HUM;
      break;
    case GROW_STATE_PROGRAMMING_DELTA_HUM :
      rotaryEnc.write(maxDehum * rotaryMultiplier);
      growState = GROW_STATE_PROGRAMMING_MAX_DEHUM;
      break;
    case GROW_STATE_PROGRAMMING_MAX_DEHUM :
      rotaryEnc.write(deltaDehum * rotaryMultiplier);
      growState = GROW_STATE_PROGRAMMING_DELTA_DEHUM;
      break;
    case GROW_STATE_PROGRAMMING_DELTA_DEHUM :
      rotaryEnc.write((maxPpm * rotaryMultiplier) / 10);
      growState = GROW_STATE_PROGRAMMING_MAX_PPM;
      break;
    case GROW_STATE_PROGRAMMING_MAX_PPM :
      rotaryEnc.write((deltaPpm * rotaryMultiplier) / 10);
      growState = GROW_STATE_PROGRAMMING_DELTA_PPM;
      break;
    case GROW_STATE_PROGRAMMING_DELTA_PPM :                         // Save settings and return to idle state
      growState = GROW_STATE_IDLE;
      rotaryEnc.write(0);
      saveSettings();
      break;
  }
}
//---------<--------------------------<--------------------------<----------------------------<-----------------------------<

//--------->-------------------------->-------------------------->----------------------------BUTTON LONG PRESS FUNCTION----------------------------->
void rotaryBtLPr() {
  wakeUpFunc();                                           // Enable backlight
  lcd.clear();
  if (growState == GROW_STATE_IDLE) {                     // Go to programming mode, if long press takes place on idle state
    growState = GROW_STATE_PROGRAMMING_MAX_TEMP;
    rotaryEnc.write(maxTemp * rotaryMultiplier);          // Set rotary position according to our next value
    loadSettings();
  } else if (growState != GROW_STATE_PANIC) {                                               // Save settings and go to iddle state, if long press takes place elsewhere
    growState = GROW_STATE_IDLE;
    rotaryEnc.write(0);
    saveSettings();
  }
}
//---------<--------------------------<--------------------------<----------------------------<-----------------------------<

//--------->-------------------------->-------------------------->----------------------------REFRESH SCREEN----------------------------->
void screenRef() {
  //lcd.clear();
  if (growState != GROW_STATE_IDLE) {                  // Print programming mode line 1
    lcd.setCursor(0, 0);
    lcd.print("Programming Mode");
  }
  switch (growState) {                                 // Check Grow Os state
    case GROW_STATE_IDLE :                             // Print idle menu
      lcd.setCursor(0, 0);                             // Print first line
      if (htOK) {                                      // If DHT22 conenction is OK, print values
        lcd.print(temp, 1);
        lcd.write(223);
        lcd.print(" ");
        lcd.print(hum, 1);
        lcd.print("% ");
      } else {                                         // If DHT22 conenction is NOT OK, print ERROR
        lcd.print("DHT22 ERROR ");
      }
      if (co2SensorOn && !co2Error) {
        lcd.print(co2Level);
        lcd.print("   ");
      }
      else if (co2Error && co2SensorOn) lcd.print("ERR ");
      else if (!co2SensorOn) lcd.print("OFF ");
      lcd.setCursor(0, 1);                             // Print second line
      lcd.print("L:");
      lcd.print(seeLight);
      lcd.print(" F:");
      if (exhaustOn) {
        if (exhaustHigh) {
          lcd.print("H");
        } else {
          lcd.print("L");
        }
      } else {
        lcd.print("0");
      }
      lcd.print(" D:");
      lcd.print(dehumOn);
      lcd.print(" C:");
      lcd.print(co2ValveOn);
      break;
    case GROW_STATE_PROGRAMMING_MAX_TEMP :              // Print max temp menu
      lcd.setCursor(0, 1);
      lcd.print("Temperature: ");
      if (blinkText) {                                  // Blink text
        lcd.print("    ");
      } else {
        lcd.print(maxTemp);
        lcd.write(223);
      }
      break;
    case GROW_STATE_PROGRAMMING_DELTA_TEMP :            // Print delta temp menu
      lcd.setCursor(0, 1);
      lcd.print(" Delta Temp:");
      if (blinkText) {
        lcd.print("    ");
      } else {
        lcd.print(deltaTemp);
        lcd.write(223);
      }
      break;
    case GROW_STATE_PROGRAMMING_MAX_HUM :               // Print max hum menu
      lcd.setCursor(0, 1);
      lcd.print(" Humidity: ");
      if (blinkText) {
        lcd.print("    ");
      } else {
        lcd.print(maxHum);
        lcd.print("%");
      }
      break;
    case GROW_STATE_PROGRAMMING_DELTA_HUM :             // Print delta hum menu
      lcd.setCursor(0, 1);
      lcd.print(" Delta Hum:");
      if (blinkText) {
        lcd.print("    ");
      } else {
        lcd.print(deltaHum);
        lcd.print("%");
      }
      break;

    case GROW_STATE_PROGRAMMING_MAX_DEHUM :             // Print max dehum menu
      lcd.setCursor(0, 1);
      lcd.print("Dehumidifier:");
      if (blinkText) {
        lcd.print("    ");
      } else {
        lcd.print(maxDehum);
        lcd.print("%");
      }
      break;
    case GROW_STATE_PROGRAMMING_DELTA_DEHUM :             // Print delta dehum menu
      lcd.setCursor(0, 1);
      lcd.print(" Delta Dehum:");
      if (blinkText) {
        lcd.print("    ");
      } else {
        lcd.print(deltaDehum);
        lcd.print("%");
      }
      break;

    case GROW_STATE_PROGRAMMING_MAX_PPM :               // Print max co2 ppm menu
      lcd.setCursor(0, 1);
      lcd.print(" CO2 PPM: ");
      if (blinkText) {
        lcd.print("    ");
      } else {
        lcd.print(maxPpm);
      }
      break;
    case GROW_STATE_PROGRAMMING_DELTA_PPM :             // Print delta co2 ppm menu
      lcd.setCursor(0, 1);
      lcd.print(" Delta PPM:");
      if (blinkText) {
        lcd.print("    ");
      } else {
        lcd.print(deltaPpm);
      }
      break;
    case GROW_STATE_PANIC :             // Print panic mode
      lcd.clear();
      wakeUpFunc();
      lcd.setCursor(0, 0);
      lcd.print("  PANIC MODE: ");
      lcd.print(panicCode);
      lcd.setCursor(0, 1);
      lcd.print("Resetting... ");
      delay(2000);
      panicCode = 0;
      rotaryEnc.write(0);
      initialize();
      break;
  }
}
//---------<--------------------------<--------------------------<----------------------------<-----------------------------<

//--------->-------------------------->-------------------------->----------------------------READ ROTARY POSITION----------------------------->
void readRotary() {
  posNewRotary = rotaryEnc.read();
  if (posNewRotary != posOldRotary) {
    wakeUpFunc();                                                      // Enable backlight
    posOldRotary = posNewRotary;
    if (posNewRotary < 0) {
      rotaryEnc.write(0);
    }
    switch (growState) {                                               // Check Grow Os state
      case GROW_STATE_PROGRAMMING_MAX_TEMP :                           // Set max temperature
        maxTemp = posNewRotary / rotaryMultiplier;
        if (maxTemp > 45) {
          maxTemp = 45;
          rotaryEnc.write(45 * rotaryMultiplier);
        } else if (maxTemp < 25) {
          maxTemp = 25;
          rotaryEnc.write(25 * rotaryMultiplier);
        }
        break;
      case GROW_STATE_PROGRAMMING_DELTA_TEMP :                         // Set delta temperature
        deltaTemp = posNewRotary / rotaryMultiplier;
        if (deltaTemp > 20) {
          deltaTemp = 20;
          rotaryEnc.write(20 * rotaryMultiplier);
        } else if (deltaTemp < 1) {
          deltaTemp = 1;
          rotaryEnc.write(rotaryMultiplier);
        }
        break;
      case GROW_STATE_PROGRAMMING_MAX_HUM :                            // Set max humidity
        maxHum = posNewRotary / rotaryMultiplier;
        if (maxHum > 90) {
          maxHum = 90;
          rotaryEnc.write(90 * rotaryMultiplier);
        } else if (maxHum < 40) {
          maxHum = 40;
          rotaryEnc.write(40 * rotaryMultiplier);
        }
        break;
      case GROW_STATE_PROGRAMMING_DELTA_HUM :                          // Set delta humidity
        deltaHum = posNewRotary / rotaryMultiplier;
        if (deltaHum > 40) {
          deltaHum = 40;
          rotaryEnc.write(40 * rotaryMultiplier);
        } else if (deltaHum < 5) {
          deltaHum = 5;
          rotaryEnc.write(5 * rotaryMultiplier);
        }
        break;
      case GROW_STATE_PROGRAMMING_MAX_DEHUM :                          // Set max dehumidifier
        maxDehum = posNewRotary / rotaryMultiplier;
        if (maxDehum > 90) {
          maxDehum = 90;
          rotaryEnc.write(40 * rotaryMultiplier);
        } else if (maxDehum < 5) {
          maxDehum = 5;
          rotaryEnc.write(5 * rotaryMultiplier);
        }
        break;
      case GROW_STATE_PROGRAMMING_DELTA_DEHUM :                          // Set delta dehumidifier
        deltaDehum = posNewRotary / rotaryMultiplier;
        if (deltaDehum > 40) {
          deltaDehum = 40;
          rotaryEnc.write(40 * rotaryMultiplier);
        } else if (deltaDehum < 5) {
          deltaDehum = 5;
          rotaryEnc.write(5 * rotaryMultiplier);
        }
        break;
      case GROW_STATE_PROGRAMMING_MAX_PPM :                            // Set max co2 ppm
        maxPpm = ((posNewRotary / rotaryMultiplier) * 10);
        if (maxPpm > 2550) {
          maxPpm = 2550;
          rotaryEnc.write(255 * rotaryMultiplier);
        } else if (maxPpm < 350) {
          maxPpm = 350;
          rotaryEnc.write(35 * rotaryMultiplier);
        }
        break;
      case GROW_STATE_PROGRAMMING_DELTA_PPM :                          // Set delta co2 ppm
        deltaPpm = ((posNewRotary / rotaryMultiplier) * 10);
        if (deltaPpm > 500) {
          deltaPpm = 500;
          rotaryEnc.write(50 * rotaryMultiplier);
        } else if (deltaPpm < 100) {
          deltaPpm = 100;
          rotaryEnc.write(10 * rotaryMultiplier);
        }
        break;

    }
  }
}
//---------<--------------------------<--------------------------<----------------------------<-----------------------------<

//--------->-------------------------->-------------------------->----------------------------LOAD SETTINGS----------------------------->
void loadSettings() {
  if (EEPROM.read(loadCheckAddr) == loadCheck) {      // Check first run
    maxTemp = EEPROM.read(maxTempAddr);               // If there are saved values load them
    deltaTemp = EEPROM.read(deltaTempAddr);
    maxHum = EEPROM.read(maxHumAddr);
    deltaHum = EEPROM.read(deltaHumAddr);
    maxPpm = EEPROM.read(maxPpmAddr) * 10;
    deltaPpm = EEPROM.read(deltaPpmAddr) * 10;
    maxDehum = EEPROM.read(maxDehumAddr);
    deltaDehum = EEPROM.read(deltaDehumAddr);
  } else {                                            // If there are no saved values, set default values and save them
    maxTemp = 30;
    deltaTemp = 10;
    maxHum = 80;
    deltaHum = 10;
    maxPpm = 1500;
    deltaPpm = 200;
    maxDehum = 60;
    deltaDehum = 10;
    saveSettings();
  }
}
//---------<--------------------------<--------------------------<----------------------------<-----------------------------<

//--------->-------------------------->-------------------------->----------------------------SAVE SETTINGS----------------------------->
void saveSettings() {
  EEPROM.update(maxTempAddr, maxTemp);
  EEPROM.update(deltaTempAddr, deltaTemp);
  EEPROM.update(maxHumAddr, maxHum);
  EEPROM.update(deltaHumAddr, deltaHum);
  EEPROM.update(maxPpmAddr, maxPpm / 10);
  EEPROM.update(deltaPpmAddr, deltaPpm / 10);
  EEPROM.update(maxDehumAddr, maxDehum);
  EEPROM.update(deltaDehumAddr, deltaDehum);
  EEPROM.update(loadCheckAddr, loadCheck);
}
//---------<--------------------------<--------------------------<----------------------------<-----------------------------<

//--------->-------------------------->-------------------------->----------------------------WAKE UP----------------------------->
void wakeUpFunc() {
  blTimeoutTS = millis();
  if (!wakeUp) {
    wakeUp = true;
    digitalWrite(blPin, HIGH);
  }
}
//---------<--------------------------<--------------------------<----------------------------<-----------------------------<

//--------->-------------------------->-------------------------->----------------------------COMPUTE RELAYS----------------------------->
void compute() {
  if (seeLight) {
    //if (!co2SensorOn) {
    // power(1);
    // initializeCo2();
    // co2SensorOn = true;
    //}
    if (!exhaustHigh) {
      exhaustHigh = true;
      digitalWrite(relayPin1, LOW);
    }
    if (temp < maxTemp - deltaTemp && hum < maxHum - deltaHum) {
      if (exhaustOn) {
        exhaustOn = false;
        digitalWrite(relayPin2, HIGH);
      }
    } else if (temp >= maxTemp || hum >= maxHum) {
      if (!exhaustOn) {
        if (co2ValveOn) {
          co2ValveOn = false;
          digitalWrite(relayPin4, HIGH);
        }
        exhaustOn = true;
        digitalWrite(relayPin2, LOW);
      }
    }
    if (!exhaustOn) {
      if (co2Level < maxPpm - deltaPpm) {
        co2ValveOn = true;
        digitalWrite(relayPin4, LOW);
      } else if (co2Level >= maxPpm) {
        co2ValveOn = false;
        digitalWrite(relayPin4, HIGH);
      }
    }
  } else {
    //if (co2SensorOn) {
    //  power(0);
    // co2SensorOn = false;
    // }
    if (co2ValveOn) {
      co2ValveOn = false;
      digitalWrite(relayPin4, HIGH);
    }
    if (!exhaustOn) {
      exhaustOn = true;
      digitalWrite(relayPin2, LOW);
    }
    if (temp < maxTemp - deltaTemp && hum < maxHum - deltaHum) {
      if (exhaustHigh) {
        exhaustHigh = false;
        digitalWrite(relayPin1, HIGH);
      }
    } else if (temp >= maxTemp || hum >= maxHum) {
      if (!exhaustHigh) {
        exhaustHigh = true;
        digitalWrite(relayPin1, LOW);
      }
    }
  }

  //dehum routine
  if (hum < maxDehum - deltaDehum) {
    if (dehumOn) {
      digitalWrite(relayPin3, HIGH);
      dehumOn = false;
    }
  } else if (hum >= maxDehum) {
    if (!dehumOn) {
      digitalWrite(relayPin3, LOW);
      dehumOn = true;
    }
  }
}
//---------<--------------------------<--------------------------<----------------------------<-----------------------------<

//--------->-------------------------->-------------------------->----------------------------CO2 INITIALIZE----------------------------->
void initialize() {
  lcd.clear();
  growState = GROW_STATE_IDLE;                  // Set current state
  loadSettings();                               // Load settings
  lcd.setCursor(5, 0);                          // Splash screen
  lcd.print("Grow OS");
  lcd.setCursor(1, 1);
  lcd.print("ver. 0.991 beta");
  delay(2000);
  co2SensorOn = true;
  delay(300);
  lcd.clear();
  for (int i = 10; i >= 1; i--) {
    lcd.setCursor(0, 0);
    lcd.print("   CO2 Sensor  ");
    lcd.setCursor(0, 1);
    lcd.print("Initializing: ");
    lcd.print(i);
    lcd.print(" ");
    delay(1000);
  }
}
//---------<--------------------------<--------------------------<----------------------------<-----------------------------<

//--------->-------------------------->-------------------------->----------------------------PANIC ROUTINE----------------------------->
void panic() {
  lcd.clear();
  growState = GROW_STATE_PANIC;
  digitalWrite(relayPin1, HIGH);                                             // Reversed relays!!!!!!!!!!!!!
  digitalWrite(relayPin2, LOW);
  digitalWrite(relayPin3, HIGH);
  digitalWrite(relayPin4, HIGH);
}
//---------<--------------------------<--------------------------<----------------------------<-----------------------------<

