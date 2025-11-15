#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include "SFE_ISL29125.h"
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

#include "points.h"

#define ENV 0
#define BLUE 1
#define YELLOW 2
#define RED 3

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
  const double envAvgR, const double envAvgG, const double envAvgB,
  const double blueBallAvgR, const double blueBallAvgG, const double blueBallAvgB,
  const double yellowBallAvgR, const double yellowBallAvgG, const double yellowBallAvgB,
  const unsigned int redMin, const unsigned int redMax,
  const unsigned int greenMin, const unsigned int greenMax,
  const unsigned int blueMin, const unsigned int blueMax){ 
  //Serial.println("called classify()");
  unsigned long start = millis();

  // counts is an array, each element tallies up num votes for each ball colour
  // counts[0] = votes for ENV -> matches macro
  // counts[1] = votes for BLUE -> matches macro
  // counts[2] = votes for YELLOW -> matches macro
  // counts[3] = votes for RED -> matches macro
  unsigned int counts[4] = { 0, 0, 0, 0 };
  
  //0-255 rgb values of a sampled datapoint
  int redVal, greenVal, blueVal; 
  //Serial.print("DBG: red before: ");
  //Serial.println(redVal);

  while ((millis() - start) < VOTING_WINDOW) { 
    light_LED(strip);
    sample_scaled_RGB(redVal, greenVal, blueVal, RGB_sensor,
    redMin, redMax,
    greenMin, greenMax,
    blueMin, blueMax);
    //Serial.print("DBG: red after (TO MAKE SURE IT CHANGES): ");
    //Serial.println(redVal);
    // redVal is the scaled R value from sensor, and same for other colours
    // (redVal, greenVal, blueVal) creates a sampled point in 3D space (with R,G,B axes)

    // compute euclidean distance btwn sampled point and ball colour point
    float distEnv = sqrt(sq(redVal - envAvgR) + sq(greenVal - envAvgG) + sq(blueVal - envAvgB));
    float distBlue = sqrt(sq(redVal - blueBallAvgR) + sq(greenVal - blueBallAvgG) + sq(blueVal - blueBallAvgB));
    float distYellow = sqrt(sq(redVal - yellowBallAvgR) + sq(greenVal - yellowBallAvgG) + sq(blueVal - yellowBallAvgB));
    //float distRed = sq(redVal - redBallAvgR) + sq(greenVal - redBallAvgG) + sq(blueVal - redBallAvgB); //COMMENTED OUT RED CUZ I DONT HAVE RED BALL HERE

    // pick shortest distance to be the output colour
    float minDist = distEnv;
    int choice = 0;  // 0=no, 1=blue, 2=yellow, 3=red
    if (distBlue < minDist) { 
      minDist = distBlue; 
      choice = 1; 
    }
    if (distYellow < minDist) { 
      minDist = distYellow; 
      choice = 2; 
    }
    //if (distRed < minDist) { minDist = distRed; choice = 3; } //COMMENTED OUT RED CUZ I DONT HAVE RED BALL HERE

    counts[choice]++;      // tally this vote
    delay(VOTING_INTERVAL);
  }

  // 3) pick the colour with the most votes
  int best = 0;
  for (int i = 1; i < 4; i++) {
    if (counts[i] > counts[best]) {
      best = i;
    }
  }

  // 4) print just one final result
  switch (best) {
    case ENV: 
      Serial.println(">>> NO BALL"); 
      break;
    case BLUE: 
      Serial.println(">>> BLUE BALL"); //blue ball earns 5 points
      points = points + 5;
      break;
    case YELLOW: 
      Serial.println(">>> YELLOW BALL"); //yellow ball earns 10 points
      points = points + 10;
      break;
    case RED: 
      Serial.println(">>> RED BALL"); 
      points = points + 20;
      break;
  }
  //display_points(points, hex_points); //usually comment this out & just display using serial last 2 lines
  //Serial.println("call display_points()");
  Serial.print("POINTS: ");
  Serial.println(points);
}
