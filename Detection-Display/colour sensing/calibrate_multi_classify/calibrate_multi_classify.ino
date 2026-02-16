/*
This program calibrates 2 things...
1) light intensity: scale raw light sensor input intensity values to RGB [0,255]
2) ball colour: gets the average scaled RGB values of different ball colours
Then classifies ball on key press
*/ 

#include <Wire.h>
#include "SFE_ISL29125.h"
#include <Adafruit_NeoPixel.h>

#define LED_PIN 6   // DIN pin of LED
#define NUM_LEDS 2  // num of LEDs chained together


#define SENSOR2_LED1 3
#define SENSOR2_LED2 4

#include "Wire.h"
#define MUX_ADDR 0x70 //TCA9548A encoder address

//helper function to help select port
// use by doing TCAsel(0) -> TCAsel(7)
void TCAsel(uint8_t i2c_bus){
  if (i2c_bus > 7){
    return;
  }

  Wire.beginTransmission (MUX_ADDR);
  Wire.write(1 << i2c_bus);
  Wire.endTransmission();
}
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

// Calibrated averages for multiple colours
const int NUM_COLORS = 10;
double colorAvgR[NUM_COLORS];
double colorAvgG[NUM_COLORS];
double colorAvgB[NUM_COLORS];

// human-friendly prompt names and code-friendly names
const char* colorNames[NUM_COLORS] = {
  "NO BALL",
  "LIGHT BLUE",
  "YELLOW",
  "WHITE",
  "DARK BLUE",
  "RED",
  "PINK",
  "PURPLE",
  "GREEN",
  "ORANGE"
};
const char* codeNames[NUM_COLORS] = {
  "env", "lightBlue", "yellow", "white", "darkBlue",
  "red", "pink", "purple", "green", "orange"
};

const unsigned long INTENSITY_CALIB_DURATION = 60000UL; // calibrate light intensity duration
const unsigned long BALL_CALIB_DURATION = 10000UL; // calibrate ball duration
const unsigned long SAMPLE_INTERVAL = 500UL; // time between samples

const unsigned long VOTING_WINDOW   = 1000UL; // voting window for each ball
const unsigned long VOTING_INTERVAL = 50UL; // how long to wait between each vote

void light_LED();
void remove_void_sample();
void light_intensity_calibration();
void sample_scaled_RGB();
void ball_colour_calibration(int idx);
void ball_colour_calibration_helper();
void classify();

/****************************SETUP & MAIN LOOP***************************/
void setup() {
  Serial.begin(9600); // terminal baud rate
  while (!Serial);    // waits for USB serial interface object to connect
  Serial.println(" ");

  TCAsel(7);
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
  TCAsel(7);
  r = RGB_sensor.readRed();
  g = RGB_sensor.readGreen();
  b = RGB_sensor.readBlue();

  // if it’s a void startup sample (all 0s), wait 1s, try in next loop() run
  while (r == 0 || g == 0 || b == 0) {
    delay(1000);
    TCAsel(7);
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
    TCAsel(7);
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
    if (b > blueMax) blueMax  = b;
    
    Serial.print("*");  // progress indicator
    delay(SAMPLE_INTERVAL); // wait between samples
  }
      
  Serial.println("\nLight Intensity Calibration Complete");

  Serial.print("const unsigned int redMin = ");
  Serial.print(redMin); Serial.println(";");
  Serial.print("const unsigned int redMax = ");
  Serial.print(redMax); Serial.println(";");

  Serial.print("const unsigned int greenMin = ");
  Serial.print(greenMin); Serial.println(";");
  Serial.print("const unsigned int greenMax = ");
  Serial.print(greenMax); Serial.println(";");

  Serial.print("const unsigned int blueMin = ");  
  Serial.print(blueMin); Serial.println(";");
  Serial.print("const unsigned int blueMax = "); 
  Serial.print(blueMax); Serial.println(";");

}

void sample_scaled_RGB() { // read & scale raw values from sensor into [0–255]
  // store raw light intensity values that sensor reads
  TCAsel(7);
  r = RGB_sensor.readRed();
  g = RGB_sensor.readGreen();
  b = RGB_sensor.readBlue();
  
  redScaled = map(r, redMin, redMax, 0, 255);
  greenScaled = map(g, greenMin, greenMax, 0, 255);
  blueScaled = map(b, blueMin, blueMax, 0, 255);

  redVal = constrain(redScaled, 0, 255);
  greenVal = constrain(greenScaled, 0, 255);
  blueVal = constrain(blueScaled, 0, 255);

  Serial.print("Raw: ");
  Serial.print(r); Serial.print(" ");
  Serial.print(g); Serial.print(" ");
  Serial.println(b);
}

void ball_colour_calibration(int idx){
  // accumulate and avg the mean R, G, B values for color index idx

  unsigned long start = millis();
  unsigned long count = 0; // counts how many samples taken
  unsigned long sumR = 0, sumG = 0, sumB = 0; // accumulate all R, G, B values

  while (millis() - start < BALL_CALIB_DURATION) {
    light_LED(); // keep lighting up LED in case of disconnection
    sample_scaled_RGB(); // red/green/blueVals all have scaled 0-255 vals

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

  colorAvgR[idx] = avgR;
  colorAvgG[idx] = avgG;
  colorAvgB[idx] = avgB;

  // no per-color print here any more; printing is done as a single array after all colors
}

void ball_colour_calibration_helper(){
  for (int i = 0; i < NUM_COLORS; ++i) {
    if (i == 0) {
      Serial.println("Ensure NO ball is visible. Press any key to begin NO BALL (environment) calibration");
    } else {
      Serial.print("Place ");
      Serial.print(colorNames[i]);
      Serial.println(" ball in front of sensor. Press any key to begin calibration");
    }
    while (!Serial.available());
    Serial.read();
    Serial.print("Calibrating ");
    Serial.println(colorNames[i]);
    ball_colour_calibration(i);
    // clear any extra serial input
    while (Serial.available()) Serial.read();
    delay(300); // small settle between calibrations
  }

  // Print single array of RGB averages in the same order as colorNames
  // Format: [[r,g,b],[r,g,b],...]
  Serial.println(F("\nCalibration array (one triple per color in order):"));
  Serial.print("[");
  for (int i = 0; i < NUM_COLORS; ++i) {
    Serial.print("[");
    Serial.print(colorAvgR[i], 4);
    Serial.print(",");
    Serial.print(colorAvgG[i], 4);
    Serial.print(",");
    Serial.print(colorAvgB[i], 4);
    Serial.print("]");
    if (i < NUM_COLORS - 1) {
      Serial.print(",");
      Serial.println();
    }
  }
  Serial.println("]");
  Serial.println("Ball Calibration complete! Entering detection loop");
}

void classify(){
  unsigned long start = millis();

  unsigned int counts[NUM_COLORS];
  for (int i = 0; i < NUM_COLORS; ++i) counts[i] = 0;

  while ((millis() - start) < VOTING_WINDOW) {

    sample_scaled_RGB();

    // compute euclidean distance between sampled point and each color average
    float minDist = -1;
    int choice = 0;
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

    counts[choice]++;      // tally this vote
    delay(VOTING_INTERVAL);
  }

  // 3) pick the colour with the most votes
  int best = 0;
  for (int i = 1; i < NUM_COLORS; i++) {
    if (counts[i] > counts[best]) best = i;
  }

  // 4) print just one final result
  Serial.print(">>> ");
  Serial.println(colorNames[best]);
}
