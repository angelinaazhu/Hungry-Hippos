
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
const float TARGET_RPM      = 800.0;                           // try 120–240 depending on torque/supply
const float MAX_SPEED       = (STEPS_PER_REV * TARGET_RPM) / 60.0; // steps/s
const float ACCEL           = 8000.0;                          // steps/s^2 (raise if your rig allows)
const unsigned int PULSE_US = 3;                               // 2–3 µs for DRV8825; 1–2 µs for A4988

AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

/****************END OF MOTOR GLOBAL VARS*****************************/

/****************SENSOR GLOBAL VARS*****************'******************/
#include <Wire.h>
#include "SFE_ISL29125.h"
#include <Adafruit_NeoPixel.h>

#define LED_PIN 6   // DIN pin of LED
#define NUM_LEDS 2  // num of LEDs chained together

#define ENV 0
#define BLUE 1
#define YELLOW 2
#define RED 3

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800); // LED object
SFE_ISL29125 RGB_sensor; // colour sensor object

// prev calculated min/max RGB intensity values
const unsigned int redMin = 71;
const unsigned int redMax = 1946;
const unsigned int greenMin = 213;
const unsigned int greenMax = 3520;
const unsigned int blueMin = 223;
const unsigned int blueMax = 1671; 

// prev calculated env & ball colour points
const double envAvgR = 8.50, envAvgG = 3.75, envAvgB = 14.00;
const double blueBallAvgR = 8.75, blueBallAvgG = 40.75, blueBallAvgB = 188.75;
const double yellowBallAvgR = 246.75, yellowBallAvgG = 241.50, yellowBallAvgB = 248.75;
const double redBallAvgR = 0.00, redBallAvgG = 0.00,  redBallAvgB = 0.00;

unsigned int r, g, b; // raw intensity values read in directly from sensor
int redScaled, greenScaled, blueScaled; // intermediate scaled RGB values
int redVal, greenVal, blueVal; // clipped ver of intemediate scaled RGB values

const unsigned long VOTING_WINDOW   = 500UL; // voting window for each ball
const unsigned long VOTING_INTERVAL = 50UL; // how long to wait between each vote
/*****************END OF SENSOR GLOBAL VARS***************************/

void spinRevs(float revs, int dir);
void light_LED();
void remove_void_sample();
void sample_scaled_RGB();
void classify();

/****************************SETUP & MAIN LOOP***************************/
void setup() {
  Serial.begin(9600); // terminal baud rate
  while (!Serial);    // waits for USB serial interface object to connect
  Serial.println(" ");

  /*********************SENSOR STUFF**********************/
  // Checks if sensor is connected properly -> if not, re-wire & re-upload code
  if (!RGB_sensor.init()) {
    Serial.println("Sensor init failed! Check wiring.");
    while (1); // stays here
  }
  
  // LED
  strip.begin();// init LED
  strip.show(); // turn off all LED initially
    // light up LED

  Serial.println("Sensor init successful.");
  remove_void_sample(); // only happens once in beginning
  /***************END OF SENSOR STUFF**********************/

  /***************MOTOR STUFF******************************/
  stepper.setMinPulseWidth(PULSE_US);
  stepper.setMaxSpeed(MAX_SPEED);
  stepper.setAcceleration(ACCEL);
  /***************END OF MOTOR STUFF***********************/
}

void loop(){
  spinRevs(1.0/4.0f, +1);   // 360° CW
  Serial.println("spin");
  delay(500);

  //spinRevs(0.0f, +1);   // 360° CW
  Serial.println("stop");
  delay(500); // split so that it stabilized for half a second then senses
  Serial.println("calling classify()");
  classify(); // internalyl delays for 500ms
}
/************************END OF SETUP & MAIN LOOP***************************/

// Spin exactly N revolutions (+1 CW, -1 CCW)
void spinRevs(float revs, int dir) {
  long steps = (long)(revs * STEPS_PER_REV + 0.5f); // round to nearest step
  stepper.moveTo(stepper.currentPosition() + dir * steps);
  while (stepper.distanceToGo() != 0) stepper.run();
}

void light_LED(){
  strip.setBrightness(255); // brightness [0-255]
  strip.setPixelColor(0, strip.Color(255, 255, 255)); //LED0: R,G,B=255,255,255
  strip.setPixelColor(1, strip.Color(255, 255, 255)); //LED1: R,G,B=255,255,255
  strip.show(); // update LED with set brightness & colour
}

void remove_void_sample(){
  // store raw light intensity values that sensor reads
  r = RGB_sensor.readRed();
  g = RGB_sensor.readGreen();
  b = RGB_sensor.readBlue();

  // if it’s a void startup sample (all 0s), wait 1s, try in next loop() run
  while (r == 0 || g == 0 || b == 0) {
    delay(1000);
    r = RGB_sensor.readRed();
    g = RGB_sensor.readGreen();
    b = RGB_sensor.readBlue();
  }
}

void sample_scaled_RGB() { // read & scale raw values from sensor into [0–255]
  // store raw light intensity values that sensor reads
  r = RGB_sensor.readRed();
  g = RGB_sensor.readGreen();
  b = RGB_sensor.readBlue();
  
  redScaled = map(r, redMin, redMax, 0, 255);
  greenScaled = map(g, greenMin, greenMax, 0, 255);
  blueScaled = map(b, blueMin, blueMax, 0, 255);

  redVal = constrain(redScaled, 0, 255);
  greenVal = constrain(greenScaled, 0, 255);
  blueVal = constrain(blueScaled, 0, 255);
}

void classify(){
  unsigned long start = millis();

  // counts is an array, each element tallies up num votes for each ball colour
  // counts[0] = votes for ENV -> matches macro
  // counts[1] = votes for BLUE -> matches macro
  // counts[2] = votes for YELLOW -> matches macro
  // counts[3] = votes for RED -> matches macro
  unsigned int counts[4] = { 0, 0, 0, 0 };

  while ((millis() - start) < VOTING_WINDOW) {
    light_LED();
    sample_scaled_RGB();
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
    if (distBlue < minDist) { minDist = distBlue; choice = 1; }
    if (distYellow < minDist) { minDist = distYellow; choice = 2; }
    //if (distRed < minDist) { minDist = distRed; choice = 3; } //COMMENTED OUT RED CUZ I DONT HAVE RED BALL HERE

    counts[choice]++;      // tally this vote
    delay(VOTING_INTERVAL);
  }

  // 3) pick the colour with the most votes
  int best = 0;
  for (int i = 1; i < 4; i++) {
    if (counts[i] > counts[best]) best = i;
  }

  // 4) print just one final result
  switch (best) {
    case ENV: Serial.println(">>> NO BALL"); break;
    case BLUE: Serial.println(">>> BLUE BALL"); break;
    case YELLOW: Serial.println(">>> YELLOW BALL"); break;
    case RED: Serial.println(">>> RED BALL"); break;
  }
}
