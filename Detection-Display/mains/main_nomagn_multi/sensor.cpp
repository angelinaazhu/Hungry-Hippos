#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include "SFE_ISL29125.h"
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

#include "points.h"

void light_LED(Adafruit_NeoPixel& strip){
  strip.setBrightness(255); // brightness [0-255]
  strip.setPixelColor(0, strip.Color(255, 255, 255)); //LED0: R,G,B=255,255,255
  strip.setPixelColor(1, strip.Color(255, 255, 255)); //LED1: R,G,B=255,255,255
  strip.show(); // update LED with set brightness & colour
}

void remove_void_sample(SFE_ISL29125& RGB_sensor){
  // store raw light intensity values that sensor reads
  unsigned int r = RGB_sensor.readRed();
  unsigned int g = RGB_sensor.readGreen();
  unsigned int b = RGB_sensor.readBlue();

  // if it’s a void startup sample (all 0s), wait 1s, try in next loop() run
  while (r == 0 || g == 0 || b == 0) {
    delay(1000);
    r = RGB_sensor.readRed();
    g = RGB_sensor.readGreen();
    b = RGB_sensor.readBlue();
  }
}

void sample_scaled_RGB(int& redVal, int& greenVal, int& blueVal, SFE_ISL29125& RGB_sensor,
  const unsigned int redMin, const unsigned int redMax,
  const unsigned int greenMin, const unsigned int greenMax,
  const unsigned int blueMin, const unsigned int blueMax) { // read & scale raw values from sensor into [0–255]
  // store raw light intensity values that sensor reads
  unsigned int r = RGB_sensor.readRed();
  unsigned int g = RGB_sensor.readGreen();
  unsigned int b = RGB_sensor.readBlue();
  
  int redScaled = map(r, redMin, redMax, 0, 255);
  int greenScaled = map(g, greenMin, greenMax, 0, 255);
  int blueScaled = map(b, blueMin, blueMax, 0, 255);

  redVal = constrain(redScaled, 0, 255);
  greenVal = constrain(greenScaled, 0, 255);
  blueVal = constrain(blueScaled, 0, 255);
}

// code stays in this function for the VOTING_WINDOW ~= 500ms -> NOTE: needs to be same as delay
void classify(const unsigned long VOTING_WINDOW, const unsigned long VOTING_INTERVAL, Adafruit_NeoPixel& strip, SFE_ISL29125& RGB_sensor, Adafruit_7segment& hex_points, int& points,
  const double colorAvgR[], const double colorAvgG[], const double colorAvgB[], int NUM_COLORS, const char* colorNames[], const int pointsPerColor[],
  const unsigned int redMin, const unsigned int redMax,
  const unsigned int greenMin, const unsigned int greenMax,
  const unsigned int blueMin, const unsigned int blueMax) {

  unsigned long start = millis();

  // dynamic counts array depending on numcolours there are
  unsigned int* counts = new unsigned int[NUM_COLORS];
  for (int i = 0; i < NUM_COLORS; ++i) counts[i] = 0;

  int redVal, greenVal, blueVal;

  while ((millis() - start) < VOTING_WINDOW) {
    light_LED(strip);
    sample_scaled_RGB(redVal, greenVal, blueVal, RGB_sensor,
      redMin, redMax, greenMin, greenMax, blueMin, blueMax);

    // compute distances to each calibrated color
    int choice = 0;
    float minDist = -1.0f;
    for (int i = 0; i < NUM_COLORS; ++i) {
      float dx = redVal   - colorAvgR[i];
      float dy = greenVal - colorAvgG[i];
      float dz = blueVal  - colorAvgB[i];
      float dist = sqrt(dx*dx + dy*dy + dz*dz);
      if (i == 0 || dist < minDist) {
        minDist = dist;
        choice = i;
      }
    }

    counts[choice]++;
    delay(VOTING_INTERVAL);
  }

  // pick best
  int best = 0;
  for (int i = 1; i < NUM_COLORS; i++) {
    if (counts[i] > counts[best]) {
      best = i;
    }
  }

  // print result and apply points
  Serial.print(">>> ");
  if (colorNames != nullptr) Serial.println(colorNames[best]);
  else Serial.println(best);

  if (pointsPerColor != nullptr) {
    points += pointsPerColor[best];
  }

  Serial.print("POINTS: ");
  Serial.println(points);

  delete[] counts;
}
