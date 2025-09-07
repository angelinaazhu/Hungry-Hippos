#include <Wire.h>
#include "SFE_ISL29125.h"  

/************LED vars***************/
#include <Adafruit_NeoPixel.h>
#define LED_PIN 6 // Pin where DIN (pin 4 of WS2812D-265) is connected
#define NUM_LEDS 2 // Number of LEDs in your strip (or single LED)
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
void light_LED(){
  strip.begin();        // Initialize LED
  strip.show();         // Turn off all LEDs initially
  strip.setBrightness(255); // Max brightness (0-255)
  strip.setPixelColor(0, strip.Color(255, 255, 255));
  strip.setPixelColor(1, strip.Color(255, 255, 255));
  strip.show();         // Update LED
}
/********END OF LED vars************/

SFE_ISL29125 RGB_sensor;

// How long to sample (millis)
const unsigned long calibrationInterval = 60000UL;  // 1 min
unsigned long startTime;
bool calibrationDone = false;

// Initialize extremes
unsigned int redMin   = 0xFFFF;
unsigned int greenMin = 0xFFFF;
unsigned int blueMin  = 0xFFFF;
unsigned int redMax   = 0;
unsigned int greenMax = 0;
unsigned int blueMax  = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println(" ");
  if (!RGB_sensor.init()) {
    Serial.println(F("Sensor init failed! Check wiring."));
    while (1);               // halt here
  }
  light_LED(); // light up LED
  Serial.println(F("Sensor init successful. Starting 30s calibration..."));
  startTime = millis();
}

void loop() {
  light_LED(); // keep trying to light up LED in case of disconnection
  if (!calibrationDone) {
    unsigned long elapsed = millis() - startTime;
    
    if (elapsed <= calibrationInterval) {
      // Read raw sensor values
      unsigned int r = RGB_sensor.readRed();
      unsigned int g = RGB_sensor.readGreen();
      unsigned int b = RGB_sensor.readBlue();

      // if it’s a void startup sample, wait and try again
      if (r == 0 && g == 0 && b == 0) {
      delay(2000);
      return;
      }

      // Update minima
      if (r < redMin)   redMin   = r;
      if (g < greenMin) greenMin = g;
      if (b < blueMin)  blueMin  = b;
      // Update maxima
      if (r > redMax)   redMax   = r;
      if (g > greenMax) greenMax = g;
      if (b > blueMax)  blueMax  = b;
      
      // Wait 2 seconds between samples (just like your basic example)
      delay(2000);
    }
    else {
      // Calibration finished—print out final ranges once
      Serial.println(F("\n=== Calibration Complete ==="));
      Serial.print(F("redLow = "));   Serial.println(redMin);
      Serial.print(F("redHigh = "));  Serial.println(redMax);
      Serial.print(F("greenLow = ")); Serial.println(greenMin);
      Serial.print(F("greenHigh = "));Serial.println(greenMax);
      Serial.print(F("blueLow = "));  Serial.println(blueMin);
      Serial.print(F("blueHigh = ")); Serial.println(blueMax);
      Serial.println(F("============================"));
      
      calibrationDone = true;
    }
  }
  // after printing, do nothing
}
