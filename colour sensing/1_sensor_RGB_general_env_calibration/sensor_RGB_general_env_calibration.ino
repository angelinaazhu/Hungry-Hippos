/******************************************************************************\  
 Calibrate_ISL29125.ino  
 Runs the ISL29125 sensor for 30 seconds, finds min/max R/G/B,  
 and prints out the values for use in your map()/constrain() ranges.  
*******************************************************************************/

#include <Wire.h>
#include "SFE_ISL29125.h"  

SFE_ISL29125 RGB_sensor;

// How long to sample (millis)
const unsigned long calibrationInterval = 60000UL;  // 30 seconds
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
  while (!Serial);           // wait for Serial on Leonardo/Micro (safe on Uno too)
  Serial.println(" ");
  if (!RGB_sensor.init()) {
    Serial.println(F("Sensor init failed! Check wiring."));
    while (1);               // halt here
  }
  Serial.println(F("Sensor init successful. Starting 30s calibration..."));
  startTime = millis();
}

void loop() {
  if (!calibrationDone) {
    unsigned long elapsed = millis() - startTime;
    
    if (elapsed <= calibrationInterval) {
      // Read raw sensor values
      unsigned int r = RGB_sensor.readRed();
      unsigned int g = RGB_sensor.readGreen();
      unsigned int b = RGB_sensor.readBlue();

      // if it’s that bogus startup sample, just wait and try again
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
