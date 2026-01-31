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

// SENSOR VARS
#define LED_PIN 6   // DIN pin of LED
#define NUM_LEDS 2  // num of LEDs chained together
// prev calculated min/max RGB intensity values
const unsigned int redMin = 63;
const unsigned int redMax = 6726;
const unsigned int greenMin = 141;
const unsigned int greenMax = 5307;
const unsigned int blueMin = 133;
const unsigned int blueMax = 6866;
// NEW: multi-color averages from your calibration output (order matches colorNames below)
const int NUM_COLORS = 10;
const double colorAvgR[NUM_COLORS] = {
  0.0000, 9.5000, 136.7000, 139.1000, 4.4000,
  75.2000, 151.4500, 31.6000, 39.6000, 195.1000
};
const double colorAvgG[NUM_COLORS] = {
  0.1500, 70.7000, 137.5500, 204.8000, 23.5000,
  14.0500, 107.4500, 31.9500, 120.5500, 89.1500
};
const double colorAvgB[NUM_COLORS] = {
  0.1000, 113.4500, 65.2500, 211.6500, 54.5000,
  17.0500, 117.7000, 60.9500, 58.8000, 47.0500
};
// human-friendly names in same order as calibration
const char* colorNames[NUM_COLORS] = {
  "NO BALL", "LIGHT BLUE", "YELLOW", "WHITE", "DARK BLUE",
  "RED", "PINK", "PURPLE", "GREEN", "ORANGE"
};
// points assigned per color (customize as desired)
const int pointsPerColor[NUM_COLORS] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9
};
const unsigned long VOTING_WINDOW   = 500UL;
const unsigned long VOTING_INTERVAL = 5UL;

// SPIN VARS
const int STEP_PIN   = 8;
const int DIR_PIN    = 9;
const int ENABLE_PIN = -1;
const int ENCODER_PIN = A0;
//const float RUN_SPEED = 2000;    // steps per second (choose what works)
const float TARGET_DELTA = 90.0; // 1/4 revolution
// const float MAXSPEED = 10000000000000000000;
// const float MAXACCEL = 10000000000000000000;

//MOTOR VARS: USED FOR BLIND #STEP SPINS
// const int STEP_PIN   = 8;
// const int DIR_PIN    = 9;
// const int ENABLE_PIN = -1; // set to your EN pin, or -1 if none

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

// MAGN VARS
//Magnetic sensor things
// int magnetStatus = 0; //value of the status register (MD, ML, MH)
//                       //tells if magnet is detected & if strength OK
int lowbyte; //raw angle 7:0
word highbyte; //raw angle 7:0 and 11:8
int rawAngle; //final raw angle number (0-4095)
float absAngle; //raw angle in degrees (360/4096 * [value between 0-4095])
float startAngle = 0.0; //starting angle
float relAngle = 0.0; //relative to startAngle: current angle -  startAngle 
float lastCheckpointAngle = 0.0; //to keep track of when last the motor stopped spinning (completed 90)
const float CHECKPOINT = 90.0; //checkpoint every 90 degrees


/*****************END OF ALL VARS***************/

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

  //initialize magn i2c
  Wire.begin(); //start i2C 
  Wire.setClock(800000L); //fast clock 800kHz freq

  checkMagnetPresence(); //check the magnet: wait here until magnet is found

  Serial.println("Welcome!"); //print a welcome message  
  Serial.println("AS5600"); //print a welcome message
  delay(3000);
  // Serial.print("start angle: ");
  // Serial.println(startAngle);
  // delay(100);

  //reads current angle again to set checkpoint for 90 degree rotation detection
  absAngle = getAbsAngle();
  startAngle = absAngle;
  relAngle = getRelativeAngle(absAngle, startAngle);
  lastCheckpointAngle = relAngle; // initialize last checkpoint angle to be relative starting angle

}

void loop(){
  if (start == false){
    mini_countdown(hex_timer, currentMillis, previousMillis, secondsUntilGo, start);
  } else {
    countdown(hex_timer, currentMillis, previousMillis, remainingSeconds);
    spinRevs(1.0/4.0f, -1, stepper, STEPS_PER_REV);   // 1/4 = spin quarter of a rev, +1 = CW
    //spinMagn(ENCODER_PIN, RUN_SPEED, TARGET_DELTA, stepper, -1); //-1 for cw
    //                           //stopped after returning from this function
    //spinMagnI2C(RUN_SPEED, TARGET_DELTA, stepper, -1, startAngle, lastCheckpointAngle);


    delay(100); // stabilizes for half a second after stopping, then senses -> responsible for half of how long it takes to stop & sense
    checkRotated90(-1, startAngle, lastCheckpointAngle, CHECKPOINT); // TODO: NEED TO CHECK THIS LOGIC
    classify(VOTING_WINDOW, VOTING_INTERVAL, strip, RGB_sensor, hex_points, points,
      colorAvgR, colorAvgG, colorAvgB, NUM_COLORS, colorNames, pointsPerColor,
      redMin, redMax, greenMin, greenMax, blueMin, blueMax);
  }
}