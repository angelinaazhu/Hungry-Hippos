
/****************MOTOR GLOBAL VARS************************************/
#include <AccelStepper.h>

// === Pins ===
const int STEP_PIN   = 8;
const int DIR_PIN    = 9;
const int ENABLE_PIN = -1;   // set to your EN pin, or -1 if none

// === Motor / Driver ===
const int  MOTOR_FULL_STEPS = 200;   // 1.8° motor = 200 steps/rev
const int  MICROSTEPS       = 32;    // <-- CHANGE to your driver setting: 1,2,4,8,16,...
const long STEPS_PER_REV    = (long)MOTOR_FULL_STEPS * MICROSTEPS;

// Motion targets
const float TARGET_RPM      = 180.0;                           // try 120–240 depending on torque/supply
const float MAX_SPEED       = (STEPS_PER_REV * TARGET_RPM) / 60.0; // steps/s
const float ACCEL           = 8000.0;                          // steps/s^2 (raise if your rig allows)
const unsigned int PULSE_US = 3;                               // 2–3 µs for DRV8825; 1–2 µs for A4988

AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

/*********************************************************************/

/****************SENSOR GLOBAL VARS***********************************/
#include <Wire.h>
#include "SFE_ISL29125.h"

SFE_ISL29125 RGB_sensor;

// Raw->[0..255] scaling bounds (keep from your last run)
const unsigned int redLow = 2128;
const unsigned int redHigh = 65535;
const unsigned int greenLow = 2523;
const unsigned int greenHigh = 65535;
const unsigned int blueLow = 1818;
const unsigned int blueHigh = 65535;

// Fixed averages (from your printout)
const float ENV_AVG_R    = 0.0f,   ENV_AVG_G    = 0.0f,   ENV_AVG_B    = 0.0f;
const float BLUE_AVG_R   = 0.70f,  BLUE_AVG_G   = 0.80f,  BLUE_AVG_B   = 1.50f;
const float YELLOW_AVG_R = 15.20f, YELLOW_AVG_G = 12.70f, YELLOW_AVG_B = 12.70f;
const float RED_AVG_R    = 7.70f, RED_AVG_G    = 3.50f,  RED_AVG_B    = 1.80f;

// Working copies (used by the classifier)
float envAvgR, envAvgG, envAvgB;
float blueBallAvgR, blueBallAvgG, blueBallAvgB;
float yellowBallAvgR, yellowBallAvgG, yellowBallAvgB;
float redBallAvgR, redBallAvgG, redBallAvgB;

// latest scaled readings
int redVal, greenVal, blueVal;
int redScaled, greenScaled, blueScaled;
/*********************************************************************/

// helper to read & scale into [0–255]
bool sampleScaledRGB() {
  unsigned int redRaw   = RGB_sensor.readRed();
  unsigned int greenRaw = RGB_sensor.readGreen();
  unsigned int blueRaw  = RGB_sensor.readBlue();

  // ignore bogus startup sample without blocking
  if (redRaw == 0 && greenRaw == 0 && blueRaw == 0) return false;

  int r = map(redRaw,   redLow,   redHigh,   0, 255);
  int g = map(greenRaw, greenLow, greenHigh, 0, 255);
  int b = map(blueRaw,  blueLow,  blueHigh,  0, 255);

  redVal   = constrain(r, 0, 255);
  greenVal = constrain(g, 0, 255);
  blueVal  = constrain(b, 0, 255);
  return true;
}

void setup() {
  Serial.begin(9600);
  /*****************SENSOR STUFF*******************/
  
  if (!RGB_sensor.init()) {
    Serial.println("Sensor init failed!");
    while (1);
  }

  // discard first conversion
  RGB_sensor.readRed();
  RGB_sensor.readGreen();
  RGB_sensor.readBlue();
  delay(100);

  // Load fixed prototypes (no calibration)
  envAvgR = ENV_AVG_R;       envAvgG = ENV_AVG_G;       envAvgB = ENV_AVG_B;
  blueBallAvgR = BLUE_AVG_R; blueBallAvgG = BLUE_AVG_G; blueBallAvgB = BLUE_AVG_B;
  yellowBallAvgR = YELLOW_AVG_R; yellowBallAvgG = YELLOW_AVG_G; yellowBallAvgB = YELLOW_AVG_B;
  redBallAvgR = RED_AVG_R;   redBallAvgG = RED_AVG_G;   redBallAvgB = RED_AVG_B;
  /*
  Serial.println("\n*** Using FIXED prototypes — no calibration ***");
  Serial.print("Env avg   = ");   Serial.print(envAvgR);   Serial.print(", "); Serial.print(envAvgG);   Serial.print(", "); Serial.println(envAvgB);
  Serial.print("Blue avg  = ");   Serial.print(blueBallAvgR);   Serial.print(", "); Serial.print(blueBallAvgG);   Serial.print(", "); Serial.println(blueBallAvgB);
  Serial.print("Yellow avg= ");   Serial.print(yellowBallAvgR); Serial.print(", "); Serial.print(yellowBallAvgG); Serial.print(", "); Serial.println(yellowBallAvgB);
  Serial.print("Red avg   = ");   Serial.print(redBallAvgR);    Serial.print(", "); Serial.print(redBallAvgG);    Serial.print(", "); Serial.println(redBallAvgB);
  */
  /********************************************************/


  /***************MOTOR STUFF******************************/
  stepper.setMinPulseWidth(PULSE_US);
  stepper.setMaxSpeed(MAX_SPEED);
  stepper.setAcceleration(ACCEL);
}
void sense() {
  //Serial.println("sense() called");
  // majority vote over a rolling 1s window, sampling every 100 ms
  static const unsigned long WINDOW_MS   = 500UL;
  static const unsigned long INTERVAL_MS = 100UL;

  static unsigned long windowStart = millis();
  static unsigned long lastSample  = 0;

  // counts[0]=no ball, [1]=blue, [2]=yellow, [3]=red
  static unsigned int counts[4] = {0, 0, 0, 0};

  unsigned long now = millis();

  // take a sample every INTERVAL_MS
  if (now - lastSample >= INTERVAL_MS) {
    if (sampleScaledRGB()) {
      float distEnv    = sq(redVal - envAvgR)        + sq(greenVal - envAvgG)        + sq(blueVal - envAvgB);
      float distBlue   = sq(redVal - blueBallAvgR)   + sq(greenVal - blueBallAvgG)   + sq(blueVal - blueBallAvgB);
      float distYellow = sq(redVal - yellowBallAvgR) + sq(greenVal - yellowBallAvgG) + sq(blueVal - yellowBallAvgB);
      float distRed    = sq(redVal - redBallAvgR)    + sq(greenVal - redBallAvgG)    + sq(blueVal - redBallAvgB);

      int choice = 0; // 0=no, 1=blue, 2=yellow, 3=red
      float minDist = distEnv;
      if (distBlue   < minDist) { minDist = distBlue;   choice = 1; }
      if (distYellow < minDist) { minDist = distYellow; choice = 2; }
      if (distRed    < minDist) { minDist = distRed;    choice = 3; }

      counts[choice]++;
    }
    lastSample = now;
  }

  // every WINDOW_MS, print one classification and reset counts
  if (now - windowStart >= WINDOW_MS) {
    int best = 0;
    for (int i = 1; i < 4; i++) {
      if (counts[i] > counts[best]) best = i;
    }

    switch (best) {
      case 0: Serial.println(F(">>> NO BALL"));     break;
      case 1: Serial.println(F(">>> BLUE BALL"));   break;
      case 2: Serial.println(F(">>> YELLOW BALL")); break;
      case 3: Serial.println(F(">>> RED BALL"));    break;
    }

    // reset for next second
    counts[0] = counts[1] = counts[2] = counts[3] = 0;
    windowStart = now;
  }
}

// Spin exactly N revolutions (+1 CW, -1 CCW)
void spinRevs(float revs, int dir) {
  long steps = (long)(revs * STEPS_PER_REV + 0.5f); // round to nearest step
  stepper.moveTo(stepper.currentPosition() + dir * steps);
  while (stepper.distanceToGo() != 0) stepper.run();
}


void loop(){
  spinRevs(1.0/4.0f, +1);   // 360° CW
  Serial.println("spinning full speed CW for 1s");
  delay(500);

  //spinRevs(0.0f, +1);   // 360° CW
  Serial.println("stopping for 1s");
  delay(500); // split so that it stabilized for half a second then senses
  sense();
  Serial.println("calling sense()");
  delay(500);
}