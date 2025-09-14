// This program Scales raw light sensor input intensity values to RGB [0,255]
// Rarely used on its own -> integrated with 2_sensor_RGB_ball_calibration

#include <Wire.h>
#include "SFE_ISL29125.h"  
#include <Adafruit_NeoPixel.h>

#define LED_PIN 6   // DIN pin of LED
#define NUM_LEDS 2  // num of LEDs chained together

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800); // LED object
SFE_ISL29125 RGB_sensor; // colour sensor object

const unsigned long CALIB_DURATION = 60000UL; // how long to calibrate for
const unsigned long SAMPLE_INTERVAL = 1000UL;   // time between samples
unsigned long START_TIME;

bool calibrationDone = false;

// initialize extremes to store min/max RGB intensity values
unsigned int redMin   = 0xFFFF; // will store min R value sensor sensed
unsigned int greenMin = 0xFFFF;
unsigned int blueMin  = 0xFFFF;
unsigned int redMax   = 0;      // will store max R value sensor sensed
unsigned int greenMax = 0;
unsigned int blueMax  = 0;

void light_LED(){
  strip.setBrightness(255); // brightness [0-255]
  strip.setPixelColor(0, strip.Color(255, 255, 255)); //LED0: R,G,B=255,255,255
  strip.setPixelColor(1, strip.Color(255, 255, 255)); //LED1: R,G,B=255,255,255
  strip.show(); // update LED with set brightness & colour
}

// setup, only happens once in beginning when uploading code
void setup() {
  Serial.begin(9600); // terminal baud rate
  while (!Serial);    // waits for USB serial interface object to connect
  Serial.println(" ");

  // Checks if sensor is connected properly -> if not, re-wire & re-upload code
  if (!RGB_sensor.init()) {
    Serial.println("Sensor init failed! Check wiring.");
    while (1); // stays here
  }
  
  // LED
  strip.begin();// init LED
  strip.show(); // turn off all LED initially
  light_LED();  // light up LED
 
  Serial.println("Sensor init successful. Starting general light intensity calibration");
  START_TIME = millis();
}

// program keeps running loop() until calibration time is up, then print min/max
void loop() {  
  light_LED(); // keep lighting up LED in case of disconnection
  
  if (!calibrationDone) {
    unsigned long elapsed = millis() - START_TIME; // how much time passed
    
    if (elapsed <= CALIB_DURATION) { // if CALIB_DURATION not up yet
      // store raw light intensity values that sensor reads
      unsigned int r = RGB_sensor.readRed();
      unsigned int g = RGB_sensor.readGreen();
      unsigned int b = RGB_sensor.readBlue();

      // if it’s a void startup sample (all 0s), wait 1s, try in next loop() run
      if (r == 0 && g == 0 && b == 0) {
        delay(1000);
        return; // returns from this loop() run
      }

      // update min
      if (r < redMin)   redMin   = r;
      if (g < greenMin) greenMin = g;
      if (b < blueMin)  blueMin  = b;
      
      // update max
      if (r > redMax)   redMax   = r;
      if (g > greenMax) greenMax = g;
      if (b > blueMax)  blueMax  = b;
      
      Serial.print("*");  // progress indicator
      delay(SAMPLE_INTERVAL); // wait between samples

    } else { // CALIB_DURATION is up
      
      Serial.println("\n=== Calibration Complete ===");
      
      Serial.print("redLow = ");
      Serial.println(redMin);
      Serial.print("redHigh = ");
      Serial.println(redMax);

      Serial.print("greenLow = ");
      Serial.println(greenMin);
      Serial.print("greenHigh = ");
      Serial.println(greenMax);

      Serial.print("blueLow = ");  
      Serial.println(blueMin);
      Serial.print("blueHigh = "); 
      Serial.println(blueMax);

      Serial.println("============================");
      
      calibrationDone = true; // tells program to stop
    }
  }
  // if calibration is done, do nothing
}
