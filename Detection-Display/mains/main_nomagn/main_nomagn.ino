#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include <AccelStepper.h>
#include "SFE_ISL29125.h"
#include <Adafruit_NeoPixel.h>

#include "vars.h"
#include "points.h"
#include "countdown.h"
#include "spin.h"
#include "sensor.h"

/***********ALL VARS************/
//GLOBAL POINT VAR
int points = 0;

//COUNTDOWN VARS
//actual countdown
const unsigned int startSeconds = 3000;  // 2 minutes
unsigned int remainingSeconds = startSeconds;

//mini pre game countdown
const unsigned int miniStartSeconds = 3; // 3, 2, 1, GO
unsigned int secondsUntilGo = miniStartSeconds;

//used for both countdowns
unsigned long previousMillis = 0;
unsigned long currentMillis;
bool start = false;

//MOTOR VARS: USED FOR BLIND #STEP SPINS
const int STEP_PIN   = 8;
const int DIR_PIN    = 9;
const int ENABLE_PIN = -1; // set to your EN pin, or -1 if none

const int  MOTOR_FULL_STEPS = 400; // full motor steps per revolution
                              // 360° per rev
                              // motor is 0.9° per step
                              // steps/rev = 360° per rev / 0.9° per step = 400
const int  MICROSTEPS = 32; // motor drive is rated 32 segments
                              //driver divides 1 full step -> 32 microsteps
const long STEPS_PER_REV = (long)MOTOR_FULL_STEPS * MICROSTEPS;
                            // driver motor microsteps per revolution
                            // 400 * 32 = 12800

const float TARGET_RPM = 1000.0; // revs/min NOT sure what this should be
                            // chatgpt: try 120–240 depending on torque/supply
                            // amazon: max torque is 1.8 N*m
const float MAX_SPEED = (STEPS_PER_REV * TARGET_RPM) / 60.0;
                            // microsteps per rev * revs per min = steps per min
                            // steps per min / 60 = steps per second = speed
const float ACCEL = 10000000000.0; // steps/s^2 NOT sure what this should be
                            // chatgpt: raise if your "rig" allows -> WHAT
const unsigned int PULSE_US = 3; // NOT sure what this should be
                            // 2–3 us for DRV8825
                            //1–2 us for A4988

// SENSOR VARS
#define LED_PIN 6   // DIN pin of LED
#define NUM_LEDS 2  // num of LEDs chained together

// prev calculated min/max RGB intensity values
const unsigned int redMin = 153;
const unsigned int redMax = 6603;
const unsigned int greenMin = 394;
const unsigned int greenMax = 4755;
const unsigned int blueMin = 320;
const unsigned int blueMax = 7000;

// prev calculated env & ball colour points
const double envAvgR = 0.10, envAvgG = 0.40, envAvgB = 0.90;
const double blueBallAvgR = 10.90, blueBallAvgG = 83.25, blueBallAvgB = 199.10;
const double yellowBallAvgR = 213.70, yellowBallAvgG = 203.90, yellowBallAvgB = 103.60;
//const double redBallAvgR = 0.00, redBallAvgG = 0.00,  redBallAvgB = 0.00;

const unsigned long VOTING_WINDOW   = 500UL; // voting window for each ball -> responsible for other half of how long it takes to stop & sense
const unsigned long VOTING_INTERVAL = 5UL; // how long to wait between each vote
/*****************END OF ALL VARS***************/

// MAGN + SPIN VARS
// const int STEP_PIN   = 8;
// const int DIR_PIN    = 9;
// const int ENABLE_PIN = -1;
// const int ENCODER_PIN = A0;
// const float RUN_SPEED = 2000;    // steps per second (choose what works)
// const float TARGET_DELTA = 90.0; // 1/4 revolution
// const float MAXSPEED = 10000000000000000000;
// const float MAXACCEL = 10000000000000000000;

//objects
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
SFE_ISL29125 RGB_sensor;
Adafruit_7segment hex_points = Adafruit_7segment();
Adafruit_7segment hex_timer = Adafruit_7segment();
AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

void setup() {
  delay(10);
  //initialize serial
  Serial.begin(9600); // terminal baud rate
  while (!Serial);    // waits for USB serial interface object to connect
  Serial.println(" ");

  //initialize hex -> QUESTION: CAN THEY BOTH BE AT SAME ADDRESS?
  //hex_points.begin(0x70); // for hex_points display
  hex_timer.begin(0x70); // for hex_timer display
  
  //initialize RGB sensor
  if (!RGB_sensor.init()) {
    Serial.println("Sensor init failed! Check wiring.");
    while (1); // stays here
  }
  
  //initialize LED
  strip.begin();// init LED
  light_LED(strip);

  Serial.println("Sensor init successful.");
  remove_void_sample(RGB_sensor); // only happens once in beginning
  
  //initialize stepper PW, max speed, accel
  stepper.setMinPulseWidth(PULSE_US);
  stepper.setMaxSpeed(MAX_SPEED);
  stepper.setAcceleration(ACCEL);
  // stepper.setMaxSpeed(MAXSPEED);   // higher than run speed
  // stepper.setAcceleration(MAXACCEL);
}

void loop(){
  if (start == false){
    mini_countdown(hex_timer, currentMillis, previousMillis, secondsUntilGo, start);
  } else {
    countdown(hex_timer, currentMillis, previousMillis, remainingSeconds);
    spinRevs(1.0/4.0f, -1, stepper, STEPS_PER_REV);   // 1/4 = spin quarter of a rev, +1 = CW
    //spinMagn(ENCODER_PIN, RUN_SPEED, TARGET_DELTA, stepper);
                              //stopped after returning from this function
    delay(100); // stabilizes for half a second after stopping, then senses -> responsible for half of how long it takes to stop & sense
    classify(VOTING_WINDOW, VOTING_INTERVAL, strip, RGB_sensor, hex_points, points,
    envAvgR, envAvgG, envAvgB,
    blueBallAvgR, blueBallAvgG, blueBallAvgG,
    yellowBallAvgR, yellowBallAvgG, yellowBallAvgB,
    redMin, redMax,
    greenMin, greenMax,
    blueMin, blueMax); // internalyl delays for 500ms -> VOTING_WINDOW
  }
}