/*
This program calibrates 2 things...
1) light intensity: scale raw light sensor input intensity values to RGB [0,255]
2) ball colour: gets the average scaled RGB values of different ball colours
Then classifies
3) TODO: create classify() to classify ball on key press
*/ 

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

unsigned int r, g, b; // raw intensity values read in directly from sensor
int redScaled, greenScaled, blueScaled; // intermediate scaled RGB values
int redVal, greenVal, blueVal; // clipped ver of intemediate scaled RGB values

// initialize extremes to store min/max RGB intensity values
unsigned int redMin   = 0xFFFF; // will store min R value sensor sensed
unsigned int greenMin = 0xFFFF;
unsigned int blueMin  = 0xFFFF;
unsigned int redMax   = 0;      // will store max R value sensor sensed
unsigned int greenMax = 0;
unsigned int blueMax  = 0;

// calibrated env & ball colour averages
double envAvgR, envAvgG, envAvgB;
double blueBallAvgR, blueBallAvgG, blueBallAvgB;
double yellowBallAvgR, yellowBallAvgG, yellowBallAvgB;
double redBallAvgR, redBallAvgG, redBallAvgB;

const unsigned long INTENSITY_CALIB_DURATION = 10000UL; // calibrate light intensity duration
const unsigned long BALL_CALIB_DURATION = 2000UL; // calibrate ball duration
const unsigned long SAMPLE_INTERVAL = 500UL; // time between samples

const unsigned long VOTING_WINDOW   = 1000UL; // voting window for each ball
const unsigned long VOTING_INTERVAL = 50UL; // how long to wait between each vote

void light_LED();
void remove_void_sample();
void light_intensity_calibration();
void sample_scaled_RGB();
void ball_colour_calibration(int colour);
void ball_colour_calibration_helper();
void classify();

/****************************SETUP & MAIN LOOP***************************/
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

  Serial.println("Sensor init successful.");
  remove_void_sample(); // only happens once in beginning

  Serial.println("Starting general light intensity calibration");
  light_intensity_calibration();
  
  Serial.println("Starting ball colour calibration");
  ball_colour_calibration_helper();

}

void loop() {
  light_LED();
  Serial.println(F("\nPress any key to begin BALL classification"));
  while (!Serial.available());
  Serial.read();
  while (Serial.available()) { Serial.read(); }

  classify();

  // then loop() repeats, waiting for the next key press
}
/************************END OF SETUP & MAIN LOOP***************************/

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

void light_intensity_calibration(){
  unsigned long start = millis();

  while ((millis() - start) < INTENSITY_CALIB_DURATION) {
    light_LED(); // keep lighting up LED in case of disconnection
    
    // store raw light intensity values that sensor reads
    r = RGB_sensor.readRed();
    g = RGB_sensor.readGreen();
    b = RGB_sensor.readBlue();

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
  }
      
  Serial.println("\nLight Intensity Calibration Complete");
  
  Serial.print("redMin = ");
  Serial.println(redMin);
  Serial.print("redMax = ");
  Serial.println(redMax);

  Serial.print("greenMin = ");
  Serial.println(greenMin);
  Serial.print("greenMax = ");
  Serial.println(greenMax);

  Serial.print("blueMin = ");  
  Serial.println(blueMin);
  Serial.print("blueMax = "); 
  Serial.println(blueMax);

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

void ball_colour_calibration(int colour){
  // accumulate and avg the mean R, G, B values of the env & diff ball colours
  // creates a point of reference in 3D space for how to define each ball colour
  // (envAvgR, envAvgG, engAvgB), and same for ball colours

  unsigned long start = millis();
  unsigned long count = 0; // counts how many samples taken
  unsigned long sumR = 0, sumG = 0, sumB = 0; // accumulate all R, G, B values

  while (millis() - start < BALL_CALIB_DURATION) { // while time passed < 30s
    light_LED(); // keep lighting up LED in case of disconnection
    sample_scaled_RGB(); // red/green/blueVals all have scaled 225 vals

    sumR += redVal; // accumulate
    sumG += greenVal;
    sumB += blueVal;
    count++;
    Serial.print("*");  // progress indicator

    delay(SAMPLE_INTERVAL);
  } // BALL_CALIB_DURATION is up

  Serial.println(" ");

  double avgR = double(sumR) / count; //avg of R: totalR / num of accumulations
  double avgG = double(sumG) / count;
  double avgB = double(sumB) / count;

  switch (colour) {
    case ENV:
      envAvgR = avgR;
      envAvgG = avgG;
      envAvgB = avgB;
      Serial.print("ENV ");
      break;
    case BLUE:
      blueBallAvgR = avgR;
      blueBallAvgG = avgG;
      blueBallAvgB = avgB;
      Serial.print("BLUE ");
      break;
    case YELLOW:
      yellowBallAvgR = avgR;
      yellowBallAvgG = avgG;
      yellowBallAvgB = avgB;
      Serial.print("YELLOW ");
      break;
    case RED:
      redBallAvgR = avgR;
      redBallAvgG = avgG;
      redBallAvgB = avgB;
      Serial.print("RED ");
      break;
  }

  Serial.print("R,G,B = ");
  Serial.print(avgR); Serial.print(", ");
  Serial.print(avgG); Serial.print(", ");
  Serial.println(avgB);

}

void ball_colour_calibration_helper(){
  // --- 1) Environment calibration ---
  Serial.println("Ensure NO ball is visible. Press any key to begin ENVIRONMENT calibration");
  while (!Serial.available());
  Serial.read();
  Serial.println("Calibrating ENVIRONMENT");
  ball_colour_calibration(ENV);

  // --- 2) BLUE Ball calibration ---
  Serial.println("Place BLUE ball in front of sensor. Press any key to begin BLUE BALL calibration");
  while (!Serial.available());
  Serial.read();
  Serial.println("Calibrating BLUE BALL");
  ball_colour_calibration(BLUE);

  // --- 3) YELLOW Ball calibration ---
  Serial.println("Place YELLOW ball in front of sensor. Press any key to begin YELLOW BALL calibration");
  while (!Serial.available());
  Serial.read();
  Serial.println("Calibrating YELLOW BALL");
  ball_colour_calibration(YELLOW);

  // --- 4) RED Ball calibration ---
  /*Serial.println("Place RED ball in front of sensor. Press any key to begin RED BALL calibration");
  while (!Serial.available());
  Serial.read();
  Serial.println("Calibrating RED BALL");
  ball_colour_calibration(RED); */

  Serial.println("Ball Calibration complete! Entering detection loop");

}

void classify(){
  //TODO
  Serial.println("classify() called");
}
