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
int points1 = 0; //player1 points
int points2 = 0; //player2 pooints

//COUNTDOWN VARS
//actual countdown
const unsigned int startSeconds = 20;  // 2 minutes (3000 s)
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
// #define LED_PIN 6   // DIN pin of LED
// #define NUM_LEDS 2  // num of LEDs chained together
const unsigned int led_pin1 = 6;
const unsigned int led_pin2 = 7; // change later
// prev calculated min/max RGB intensity values
const unsigned int redMin_1 = 39;
const unsigned int redMax_1 = 1221;
const unsigned int greenMin_1 = 195;
const unsigned int greenMax_1 = 1097;
const unsigned int blueMin_1 = 72;
const unsigned int blueMax_1 = 998;

const unsigned int redMin_2 = 24;
const unsigned int redMax_2 = 781;
const unsigned int greenMin_2 = 57;
const unsigned int greenMax_2 = 1862;
const unsigned int blueMin_2 = 42;
const unsigned int blueMax_2 = 735;

// NEW: multi-color averages from your calibration output (order matches colorNames below)
const int NUM_COLORS = 8;

//sensor1
const double colorAvgR_1[NUM_COLORS] = {
  5.0000,
 18.0000,
  130.9000,
  //132.9000
  11.3000,
  57.0000,
  //159.2000,
  37.0000,
  48.0000,
  239.0000
};
const double colorAvgG_1[NUM_COLORS] = {
  19.0000,
  81.0000,
  203.5000,
  //253.4500,
  39.1500,
  47.1000,
  //173.8500,
  61.9000,
  144.0000,
  169.0000
};
const double colorAvgB_1[NUM_COLORS] = {
  17.0000,
  161.2500,
  94.3000,
  //254.0500,
  88.0000,
  39.0000,
  //196.4500,
  114.0000,
  83.0000,
  93.0000
};

//sensor2
const double colorAvgR_2[NUM_COLORS] = {
  7.0000,
 25.0000,
  232.5500,
  //219.5500
  19.0500,
  66.2000,
  //206.7500
 46.3000,
  75.8000,
  245.2000
};
const double colorAvgG_2[NUM_COLORS] = {
  11.0000,
  78.7500,
  240.0000,
  //255.0000,
  36.0000,
  25.0000,
  //146.3500,
  47.8000,
  156.9000,
  112.6500
};
const double colorAvgB_2[NUM_COLORS] = {
  8.0000,
 161.3500,
  104.0000,
  //255.0000,
  86.1000,
  19.0000,
  //175.6500,
 95.3000,
  81.5500,
  63.7500
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
  -5,
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
const int STEPPER_STEPPIN_2 = 11;
const int STEPPER_DIRPIN_2 = 12; //purple
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

const float TARGET_RPM = 500.0; // revs/min NOT sure what this should be
                            // chatgpt: try 120–240 depending on torque/supply
                            // amazon: max torque is 1.8 N*m
const float MAX_SPEED = (STEPS_PER_REV * TARGET_RPM) / 60.0;
                            // microsteps per rev * revs per min = steps per min
                            // steps per min / 60 = steps per second = speed
const float ACCEL = 10000.0; // 2500 very slow but no jolt
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
// #define SENSOR2_LED1 3
// #define SENSOR2_LED2 4

//MUX VARS
const int player1 = 3; //TCA channel for player 1 (sensor 1)
const int player2 = 7; //TCA channel for player 2 (sensor 2)
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
//Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
SFE_ISL29125 RGB_sensor1;
SFE_ISL29125 RGB_sensor2;
Adafruit_7segment hex_points1 = Adafruit_7segment();
Adafruit_7segment hex_timer1 = Adafruit_7segment();
Adafruit_7segment hex_points2 = Adafruit_7segment();
Adafruit_7segment hex_timer2 = Adafruit_7segment();
AccelStepper stepper1(AccelStepper::DRIVER, STEPPER_STEPPIN_2, STEPPER_DIRPIN_2);
AccelStepper stepper2(AccelStepper::DRIVER, STEPPER_STEPPIN_1, STEPPER_DIRPIN_1);
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
  hex_timer1.begin(0x71); // for hex_timer1 display
  hex_points1.begin(0x72); // for hex_timer2 display
  hex_timer2.begin(0x73); // for hex_points1 display
  hex_points2.begin(0x75); // for hex_points2 display
  
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
  //strip.begin();// init LED
  //initialize LED
  pinMode(led_pin1, OUTPUT);
  pinMode(led_pin2, OUTPUT);
  //light up led
  light_LED(led_pin1); // lights it on pin1
  light_LED(led_pin2); // lights it on pin2

  // //initialize LED (sensor2, way2)
  // pinMode(SENSOR2_LED1, OUTPUT);
  // digitalWrite(SENSOR2_LED1, HIGH);
  // pinMode(SENSOR2_LED2, OUTPUT);
  // digitalWrite(SENSOR2_LED2, HIGH);

  Serial.println("RGB sensors 1 & 2 init successful.");

  /*** REMOVE VOID SAMPLES ***/
  TCAsel(player1);
  remove_void_sample(RGB_sensor1); // only happens once in beginning
  TCAsel(player2);
  remove_void_sample(RGB_sensor2); // only happens once in beginning

  /*** INITIALIZE STEPPER ***/
  //Set direction pins to OUTPUT mode explicitly
  pinMode(STEPPER_DIRPIN_1, OUTPUT);
  pinMode(STEPPER_DIRPIN_2, OUTPUT);
  pinMode(STEPPER_STEPPIN_1, OUTPUT);
  pinMode(STEPPER_STEPPIN_2, OUTPUT);
  
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
  lcd.setRGB(255, 255, 255);

  // TCAsel(player1);
  // startingMessage(lcd);

}

void loop(){
  if (start == false){
    //mini_countdown(hex_timer, currentMillis, previousMillis, secondsUntilGo, start);
    TCAsel(player1);
    start = startingMessage(lcd);
  } else {
    TCAsel(4);
    gameOver = countdown(hex_timer1, hex_timer2, currentMillis, previousMillis, remainingSeconds);
      
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
      //Serial.println("spun motor1");

      //Serial.println("spinning motor2");
      spinRevs(1.0/4.0f, direction, stepper2, STEPS_PER_REV);
      //Serial.println("spun motor2");
         // 1/4 = spin quarter of a rev, +1 = CW
      //spinMagn(ENCODER_PIN, RUN_SPEED, TARGET_DELTA, stepper, -1); //-1 for cw
      //                           //stopped after returning from this function
      //spinMagnI2C(RUN_SPEED, TARGET_DELTA, stepper, -1, startAngle_1, lastCheckpointAngle_1);


      delay(100); // stabilizes for half a second after stopping, then senses -> responsible for half of how long it takes to stop & sense
      //checkRotated90(-1, startAngle_1, lastCheckpointAngle_1, CHECKPOINT); // TODO: NEED TO CHECK THIS LOGIC

      //Q: IS THIS DELAY(100) NEEDED? maybe reduce or remove totally as causing hex issues
      
      TCAsel(player1);
      classify(VOTING_WINDOW, VOTING_INTERVAL, RGB_sensor1, player1,hex_points1, points1,
        colorAvgR_1, colorAvgG_1, colorAvgB_1, NUM_COLORS, colorNames, pointsPerColor,
        redMin_1, redMax_1, greenMin_1, greenMax_1, blueMin_1, blueMax_1, led_pin1);
      TCAsel(player2);
      classify(VOTING_WINDOW, VOTING_INTERVAL, RGB_sensor2, player2, hex_points2, points2,
        colorAvgR_2, colorAvgG_2, colorAvgB_2, NUM_COLORS, colorNames, pointsPerColor,
        redMin_2, redMax_2, greenMin_2, greenMax_2, blueMin_2, blueMax_2, led_pin2);


          /*
          hex_timer1.begin(0x71); // for hex_timer1 display
          hex_timer2.begin(0x72); // for hex_timer2 display
          hex_points1.begin(0x75); // for hex_points1 display
          hex_points2.begin(0x73); // for hex_points2 display
          */
    }
  }
}