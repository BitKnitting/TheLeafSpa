// Include Bluetooth

#include <Arduino.h>
#include <SPI.h>
#if not defined (_VARIANT_ARDUINO_DUE_X_) && not defined (_VARIANT_ARDUINO_ZERO_)
#include <SoftwareSerial.h>
#endif

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"

// Include NeoPixel
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

// Include
#include "ArdPrintf.h"

// Config
#define FACTORYRESET_ENABLE      1

// Bluetooth
// ...hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

// Neopixel
#define PIN            6   /* Pin used to drive the NeoPixels */

#define MAXCOMPONENTS  4
uint8_t *pixelBuffer = NULL;
uint8_t width = 0;
uint8_t height = 0;
uint8_t components = 3;     // only 3 and 4 are valid values
uint8_t stride;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel();

void setup()
{
  while (!Serial);  // required for Flora & Micro
  delay(500);

  Serial.begin(115200);
  Serial.println(F("Adafruit Bluefruit Neopixel Test"));
  Serial.println(F("------------------------------------"));

  // Initialise the module
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  // Factory Reset
  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ) {
      error(F("Couldn't factory reset"));
    }
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  /* Wait for a connection before starting the test */
  Serial.println("Waiting for a BLE connection to continue ...");

  ble.verbose(false);  // debug info is a little annoying after this point!

  while ( !ble.isConnected() )
  {
    delay(10);
  }
  
  // Wait for the connection to complete
  delay(1000);

  Serial.println(F("CONNECTED!"));
  Serial.println(F("**********"));

  // Set module to DATA mode
  Serial.println( F("Switching to DATA mode!") );
  ble.setMode(BLUEFRUIT_MODE_DATA);

  Serial.println(F("******************************"));

  // Neopixels
  pixels.begin();
}

void loop()
{
  // Echo received data
  while ( ble.isConnected() )
  {
    int command = ble.read();

    switch (command) {
      case 'V': {   // Get Version
          commandVersion();
          break;
        }
  
      case 'S': {   // Setup dimensions, components, stride...
          commandSetup();
          break;
       }

      case 'C': {   // Clear with color
          commandClearColor();
          break;
      }

      case 'B': {   // Set Brightness
          commandSetBrightness();
          break;
      }
            
      case 'P': {   // Set Pixel
          commandSetPixel();
          break;
      }
  
      case 'I': {   // Receive new image
          commandImage();
          break;
       }

    }
  }
}

void swapBuffers()
{
  uint8_t *base_addr = pixelBuffer;
  int pixelIndex = 0;
  for (int j = 0; j < height; j++)
  {
    for (int i = 0; i < width; i++) {
      if (components == 3) {
        pixels.setPixelColor(pixelIndex, pixels.Color(*base_addr, *(base_addr+1), *(base_addr+2)));
      }
      else {
        Serial.println(F("TODO: implement me"));
      }
      base_addr+=components;
      pixelIndex++;
    }
    pixelIndex += stride - width;   // move pixelIndex to the next row (take into account the stride)
  }
  pixels.show();

}

void commandVersion() {
  Serial.println(F("Command: Version check"));
  sendResponse("Neopixel v1.0");
}

void commandSetup() {
  Serial.println(F("Command: Setup"));

  width = ble.read();
  height = ble.read();
  components = ble.read();
  stride = ble.read();
  neoPixelType pixelType;
  pixelType = ble.read();
  pixelType += ble.read()<<8;
  
  ardprintf("\tsize: %dx%d", width, height);
  ardprintf("\tcomponents: %d", components);
  ardprintf("\tstride: %d", stride);
  ardprintf("\tpixelType %d", pixelType );


  if (pixelBuffer != NULL) {
      delete[] pixelBuffer;
  }

  uint32_t size = width*height;
  pixelBuffer = new uint8_t[size*components];
  pixels.updateLength(size);
  pixels.updateType(pixelType);
  pixels.setPin(PIN);

  // Done
  sendResponse("OK");
}

void commandSetBrightness() {
  Serial.println(F("Command: SetBrightness"));

   // Read value
  uint8_t brightness = ble.read();

  // Set brightness
  pixels.setBrightness(brightness);

  // Refresh pixels
  swapBuffers();

  // Done
  sendResponse("OK");
}

void commandClearColor() {
  Serial.println(F("Command: ClearColor"));

  // Read color
  uint8_t color[MAXCOMPONENTS];
  for (int j = 0; j < components;) {
    if (ble.available()) {
      color[j] = ble.read();
      j++;
    }
  }

  // Set all leds to color
  int size = width * height;
  uint8_t *base_addr = pixelBuffer;
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < components; j++) {
      *base_addr = color[j];
      base_addr++;
    }
  }

  // Swap buffers
  Serial.println(F("ClearColor completed"));
  swapBuffers();


  if (components == 3) {
    ardprintf("\tcolor (%d, %d, %d)", color[0], color[1], color[2] );
  }
  
  // Done
  sendResponse("OK");
}

void commandSetPixel() {
  Serial.println(F("Command: SetPixel"));

  // Read position
  uint8_t x = ble.read();
  uint8_t y = ble.read();

  // Read colors
  uint32_t pixelIndex = y*width+x;
  uint32_t pixelComponentOffset = pixelIndex*components;
  uint8_t *base_addr = pixelBuffer+pixelComponentOffset;
  for (int j = 0; j < components;) {
    if (ble.available()) {
      *base_addr = ble.read();
      base_addr++;
      j++;
    }
  }

  // Set colors
  if (components == 3) {
    uint32_t pixelIndex = y*stride+x;
    pixels.setPixelColor(pixelIndex, pixels.Color(pixelBuffer[pixelComponentOffset], pixelBuffer[pixelComponentOffset+1], pixelBuffer[pixelComponentOffset+2]));

    ardprintf("\tcolor (%d, %d, %d)", pixelBuffer[pixelComponentOffset], pixelBuffer[pixelComponentOffset+1], pixelBuffer[pixelComponentOffset+2] );
  }
  else {
    Serial.println(F("TODO: implement me"));
  }
  pixels.show();

  // Done
  sendResponse("OK");
}

void commandImage() {
  ardprintf("Command: Image %dx%d, %d, %d", width, height, components, stride);
  
  // Receive new pixel buffer
  int size = width * height;
  uint8_t *base_addr = pixelBuffer;
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < components;) {
      if (ble.available()) {
        *base_addr = ble.read();
        base_addr++;
        j++;
      }
    }

/*
    if (components == 3) {
      uint32_t index = i*components;
      ardprintf("\tp%d (%d, %d, %d)", i, pixelBuffer[index], pixelBuffer[index+1], pixelBuffer[index+2] );
    }
    */
  }

  // Swap buffers
  Serial.println(F("Image received"));
  swapBuffers();

  // Done
  sendResponse("OK");
}

void sendResponse(char *response) {
    ardprintf("Send Response: %s", response);
    ble.write(response, strlen(response)*sizeof(char));
}

// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

