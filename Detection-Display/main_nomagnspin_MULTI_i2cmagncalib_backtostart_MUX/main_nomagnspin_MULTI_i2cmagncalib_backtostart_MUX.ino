#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include <AccelStepper.h>
#include "SFE_ISL29125.h"
#include <Adafruit_NeoPixel.h>
#include <stdlib.h>
#include <Arduino.h>
#include "rgb_lcd.h"

#include "vars.h"
#include "points.h"
#include "countdown.h"
#include "spin.h"
#include "sensor.h"
#include "lcd.h"
#include "mux.h"

/***********ALL VARS************/
//GLOBAL POINT VAR
int points = 0;

//COUNTDOWN VARS
//actual countdown
const unsigned int startSeconds = 300;  // 2 minutes (3000 s)
unsigned int remainingSeconds = startSeconds;

//mini pre game countdown
const unsigned int miniStartSeconds = 3; // 3, 2, 1, GO
unsigned int secondsUntilGo = miniStartSeconds;

//used for both countdowns
unsigned long previousMillis = 0;
unsigned long currentMillis;
bool start = false;
bool gameOver = false;

// SENSOR VARS
#define LED_PIN 6   // DIN pin of LED
#define NUM_LEDS 2  // num of LEDs chained together
// prev calculated min/max RGB intensity values
const unsigned int redMin = 48;
const unsigned int redMax = 4683;
const unsigned int greenMin = 126;
const unsigned int greenMax = 2853;
const unsigned int blueMin = 141;
const unsigned int blueMax = 5191;

// NEW: multi-color averages from your calibration output (order matches colorNames below)
const int NUM_COLORS = 10;
const double colorAvgR[NUM_COLORS] = {
  0.0000,
  5.0000,
  221.9500,
  //139.1000,
  2.3000,
  102.8500,
  //151.4500,
  41.0000,
  58.5000,
  246.3500
};
const double colorAvgG[NUM_COLORS] = {
  0.0500,
  39.3000,
  147.9500,
  //204.8000,
  12.9000,
  17.0500,
  //107.4500,
  23.4500,
  111.5000,
  70.2000
};
const double colorAvgB[NUM_COLORS] = {
  0.0000,
  223.3500,
  116.6000,
  //211.6500,
  115.4500,
  22.6500,
  //117.7000,
  135.9500,
  98.5000,
  62.2500
};
// human-friendly names in same order as calibration
const char* colorNames[NUM_COLORS] = {
  "NO BALL",
  "LIGHT BLUE",
  "YELLOW",
  //"WHITE",
  "DARK BLUE",
  "RED",
  //"PINK",
  "PURPLE",
  "GREEN",
  "ORANGE"
};
// points assigned per color (customize as desired)
const int pointsPerColor[NUM_COLORS] = {
  0,
  5,
  5,
  //5,
  5,
  -10,
  //5,
  5,
  10,
  5
};
const unsigned long VOTING_WINDOW   = 500UL;
const unsigned long VOTING_INTERVAL = 5UL;

// SPIN VARS
const int STEPPER_STEPPIN_1   = 8;
const int STEPPER_DIRPIN_1    = 9; //purple
const int STEPPER_STEPPIN_2 = 10;
const int STEPPER_DIRPIN_2 = 13; //purple
const int ENABLE_PIN = -1;
const int ENCODER_PIN = A0;
const float RUN_SPEED = 2000;    // steps per second (choose what works)
const float TARGET_DELTA = 90.0; // 1/4 revolution
// const float MAXSPEED = 10000000000000000000;
// const float MAXACCEL = 10000000000000000000;

//MOTOR VARS: USED FOR BLIND #STEP SPINS
// const int STEPPER_STEPPIN_1   = 8;
// const int STEPPER_DIRPIN_1    = 9;
// const int ENABLE_PIN = -1; // set to your EN pin, or -1 if none

const int  MOTOR_FULL_STEPS = 200; // full motor steps per revolution
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
const float ACCEL = 2500.0; // steps/s^2 NOT sure what this should be
                            // chatgpt: raise if your "rig" allows -> WHAT
const unsigned int PULSE_US = 3; // NOT sure what this should be
                            // 2–3 us for DRV8825
                            //1–2 us for A4988
const int direction = -1; //ccw = -1, cw = 1

// MAGN VARS
//Magnetic sensor things
// int magnetStatus = 0; //value of the status register (MD, ML, MH)
//                       //tells if magnet is detected & if strength OK
int lowbyte; //raw angle 7:0
word highbyte; //raw angle 7:0 and 11:8
int rawAngle; //final raw angle number (0-4095)
float absAngle_1; //raw angle in degrees (360/4096 * [value between 0-4095])
float startAngle_1 = 0.0; //starting angle
float relAngle_1 = 0.0; //relative to startAngle_1: current angle -  startAngle_1 
float lastCheckpointAngle_1 = 0.0; //to keep track of when last the motor stopped spinning (completed 90)

float absAngle_2; //raw angle in degrees (360/4096 * [value between 0-4095])
float startAngle_2 = 0.0; //starting angle
float relAngle_2 = 0.0; //relative to startAngle_1: current angle -  startAngle_1 
float lastCheckpointAngle_2 = 0.0; //to keep track of when last the motor 
const float CHECKPOINT = 90.0; //checkpoint every 90 degrees

// FROSTED LED VARS
#define SENSOR2_LED1 3
#define SENSOR2_LED2 4

//MUX VARS
const int player1 = 2; //TCA channel for player 1 (sensor 1)
const int player2 = 5; //TCA channel for player 2 (sensor 2)
/*****************END OF ALL VARS***************/


/*MUX TODO:
- add TCAsel function (DONE)
- 2 colour sensors: instantiate 2, call everything 2x (DONE)
- 2 magnet sensors (DONE)
- 2 motors
- 4 hexes (worry about this later)
- 2 groves (didnt even integrate yet, worry about this later)
*/

//objects
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
SFE_ISL29125 RGB_sensor1;
SFE_ISL29125 RGB_sensor2;
Adafruit_7segment hex_points = Adafruit_7segment();
Adafruit_7segment hex_timer = Adafruit_7segment();
AccelStepper stepper1(AccelStepper::DRIVER, STEPPER_STEPPIN_1, STEPPER_DIRPIN_1);
AccelStepper stepper2(AccelStepper::DRIVER, STEPPER_STEPPIN_2, STEPPER_DIRPIN_2);
rgb_lcd lcd;

void setup() {
  delay(100);
  //initialize serial
  Serial.begin(9600); // terminal baud rate
  while (!Serial);    // waits for USB serial interface object to connect
  Serial.println(" ");

  //initialize hex -> QUESTION: CAN THEY BOTH BE AT SAME ADDRESS?
  //hex_points.begin(0x70); // for hex_points display
  
  /***INITIALIZE HEX***/
  hex_timer.begin(0x70); // for hex_timer display
  
  /***INITIALIZE RGB SENSORS***/
  TCAsel(player1); // select 2 I2C bus for sensor 1
  //initialize RGB sensor
  while (!RGB_sensor1.init()) {
    Serial.println("RGB sensor1 init failed! Check wiring.");
    delay(100);
  }

  TCAsel(player2); // select 5 I2C bus for sensor 2
  while (!RGB_sensor2.init()) {
    Serial.println("RGB sensor2 init failed! Check wiring.");
    delay(100);
  }
  
  /*** INITIALIZE LEDS ***/
  Serial.println("Lighting up LEDs...");
  //initialize LED (sensor1, way1)
  strip.begin();// init LED
  light_LED(strip);

  //initialize LED (sensor2, way2)
  pinMode(SENSOR2_LED1, OUTPUT);
  digitalWrite(SENSOR2_LED1, HIGH);
  pinMode(SENSOR2_LED2, OUTPUT);
  digitalWrite(SENSOR2_LED2, HIGH);

  Serial.println("RGB sensors 1 & 2 init successful.");

  /*** REMOVE VOID SAMPLES ***/
  TCAsel(player1);
  remove_void_sample(RGB_sensor1); // only happens once in beginning
  TCAsel(player2);
  remove_void_sample(RGB_sensor2); // only happens once in beginning

  /*** INITIALIZE STEPPER ***/
  //initialize stepper PW, max speed, accel
  stepper1.setMinPulseWidth(PULSE_US);
  stepper1.setMaxSpeed(MAX_SPEED);
  stepper1.setAcceleration(ACCEL);
  // stepper.setMaxSpeed(MAXSPEED);   // higher than run speed
  // stepper.setAcceleration(MAXACCEL);
  stepper2.setMinPulseWidth(PULSE_US);
  stepper2.setMaxSpeed(MAX_SPEED);
  stepper2.setAcceleration(ACCEL);

  /*** INITIALIZE MAGNETIC SENSORS ***/
  //initialize magn i2c
  Wire.begin(); //start i2C 
  Wire.setClock(800000L); //fast clock 800kHz freq

  /*checkMagnetPresence(); //check the magnet: wait here until magnet is found

  Serial.println("Welcome!"); //print a welcome message  
  Serial.println("AS5600"); //print a welcome message
  delay(3000);*/
  
  /*** CHECK AS5600 SENSORS & MAGNET ***/
  TCAsel(player1);
  while (!as5600Connected()) {
    Serial.println("AS5600 sensor1 not found. Check wiring.");
    delay(100);
  }
  TCAsel(player2);
  while (!as5600Connected()) {
    Serial.println("AS5600 sensor2 not found. Check wiring.");
    delay(100);
  }
  Serial.println("AS5600 wired.");

  TCAsel(player1);
  while (!isMagnetOK()) {
    Serial.println("Magnet1 not found.");
    delay(100);
  }
  TCAsel(player2);
  while (!isMagnetOK()) {
    Serial.println("Magnet2 not found.");
    delay(100);
  }
  Serial.println("Magnets 1 & 2 in position.");

  Serial.print("start angle: ");
  Serial.println(startAngle_1);
  delay(100);

  TCAsel(player1);
  //reads current angle again to set checkpoint for 90 degree rotation detection
  absAngle_1 = getAbsAngle();
  startAngle_1 = absAngle_1;
  relAngle_1 = getRelativeAngle(absAngle_1, startAngle_1);
  lastCheckpointAngle_1 = relAngle_1; // initialize last checkpoint angle to be relative starting angle

  TCAsel(player2);
  //reads current angle again to set checkpoint for 90 degree rotation detection
  absAngle_2 = getAbsAngle();
  startAngle_2 = absAngle_2;
  relAngle_2 = getRelativeAngle(absAngle_2, startAngle_2);
  lastCheckpointAngle_2 = relAngle_2; // initialize last checkpoint angle to be relative starting angle


  //initialize LCD
  TCAsel(player1);
  lcd.begin(16, 2);
  lcd.setRGB(250, 250, 250);

  // TCAsel(player1);
  // startingMessage(lcd);

}

void loop(){
  if (start == false){
    //mini_countdown(hex_timer, currentMillis, previousMillis, secondsUntilGo, start);
    TCAsel(player1);
    start = startingMessage(lcd);
  } else {
    gameOver = countdown(hex_timer, currentMillis, previousMillis, remainingSeconds);
      
    if (gameOver){
      TCAsel(player1);
      coolDownMsg(lcd);
      
      //spin back to start
      Serial.println("Spinning back to start...");
      TCAsel(player1);
      spinBackToZero(stepper1, direction, 0.5, RUN_SPEED, startAngle_1); // 0.5 degree tolerance
      TCAsel(player2);
      spinBackToZero(stepper2, direction, 0.5, RUN_SPEED, startAngle_2); // 0.5 degree tolerance
      //JAN31: check points1 vs points2
            //JAN31: if points1>points2
                   //JAN31: win(rgb_lcd& lcd1)
                   //JAN31: loss(rgb_lcd& lcd2)
            //JAN31: else if points2>points1
                   //JAN31: win(rgb_lcd& lcd2)
                   //JAN31: loss(rgb_lcd& lcd1)
      while(1){}; //stay here
      
    } else {
      //Serial.println("Displaying game screen");
      TCAsel(player1);
      gameScreen(lcd);

      //Serial.println("spinning motor1");
      spinRevs(1.0/4.0f, direction, stepper1, STEPS_PER_REV);
      Serial.println("spun motor1");

      //Serial.println("spinning motor2");
      spinRevs(1.0/4.0f, direction, stepper2, STEPS_PER_REV);
      Serial.println("spun motor2");
         // 1/4 = spin quarter of a rev, +1 = CW
      //spinMagn(ENCODER_PIN, RUN_SPEED, TARGET_DELTA, stepper, -1); //-1 for cw
      //                           //stopped after returning from this function
      //spinMagnI2C(RUN_SPEED, TARGET_DELTA, stepper, -1, startAngle_1, lastCheckpointAngle_1);


      delay(100); // stabilizes for half a second after stopping, then senses -> responsible for half of how long it takes to stop & sense
      //checkRotated90(-1, startAngle_1, lastCheckpointAngle_1, CHECKPOINT); // TODO: NEED TO CHECK THIS LOGIC
      
      TCAsel(player1);
      classify(VOTING_WINDOW, VOTING_INTERVAL, strip, RGB_sensor1, player1,hex_points, points,
        colorAvgR, colorAvgG, colorAvgB, NUM_COLORS, colorNames, pointsPerColor,
        redMin, redMax, greenMin, greenMax, blueMin, blueMax);
      TCAsel(player2);
      classify(VOTING_WINDOW, VOTING_INTERVAL, strip, RGB_sensor2, player2, hex_points, points,
        colorAvgR, colorAvgG, colorAvgB, NUM_COLORS, colorNames, pointsPerColor,
        redMin, redMax, greenMin, greenMax, blueMin, blueMax);
    }
  }
}