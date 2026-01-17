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

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800); // LED object
SFE_ISL29125 RGB_sensor; // colour sensor object

unsigned int r, g, b; // raw intensity values read in directly from sensor
int redScaled, greenScaled, blueScaled; // intermediate scaled RGB values
int redVal, greenVal, blueVal; // clipped ver of intemediate scaled RGB values

// initialize extremes to store min/max RGB intensity values
// unsigned int redMin   = 0xFFFF; // will store min R value sensor sensed
// unsigned int greenMin = 0xFFFF;
// unsigned int blueMin  = 0xFFFF;
// unsigned int redMax   = 0;      // will store max R value sensor sensed
// unsigned int greenMax = 0;
// unsigned int blueMax  = 0; //COMMENT BACK IN IF NEED THIS
const unsigned int redMin = 75;
const unsigned int redMax = 8890;
const unsigned int greenMin = 162;
const unsigned int greenMax = 6365;
const unsigned int blueMin = 160;
const unsigned int blueMax = 9174;


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

const unsigned long INTENSITY_CALIB_DURATION = 120000UL; // calibrate light intensity duration
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
  //light_intensity_calibration(); //COMMENT BACK IN IF NEED TO DO THIS
  
//   Serial.println("Starting ball colour calibration");
//   ball_colour_calibration_helper();

}

void loop() {
  light_LED();
  collect_and_output_rgb_samples();
  while(1);

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

//COMMENT BACK IN IF NEED TO DO T HIS
// void light_intensity_calibration(){
//   unsigned long start = millis();

//   while ((millis() - start) < INTENSITY_CALIB_DURATION) {
//     light_LED(); // keep lighting up LED in case of disconnection
    
//     // store raw light intensity values that sensor reads
//     r = RGB_sensor.readRed();
//     g = RGB_sensor.readGreen();
//     b = RGB_sensor.readBlue();

//     // update min
//     if (r < redMin)   redMin   = r;
//     if (g < greenMin) greenMin = g;
//     if (b < blueMin)  blueMin  = b;
    
//     // update max
//     if (r > redMax)   redMax   = r;
//     if (g > greenMax) greenMax = g;
//     if (b > blueMax) blueMax  = b;
    
//     Serial.print("*");  // progress indicator
//     delay(SAMPLE_INTERVAL); // wait between samples
//   }
      
//   Serial.println("\nLight Intensity Calibration Complete");

//   Serial.print("const unsigned int redMin = ");
//   Serial.print(redMin); Serial.println(";");
//   Serial.print("const unsigned int redMax = ");
//   Serial.print(redMax); Serial.println(";");

//   Serial.print("const unsigned int greenMin = ");
//   Serial.print(greenMin); Serial.println(";");
//   Serial.print("const unsigned int greenMax = ");
//   Serial.print(greenMax); Serial.println(";");

//   Serial.print("const unsigned int blueMin = ");  
//   Serial.print(blueMin); Serial.println(";");
//   Serial.print("const unsigned int blueMax = "); 
//   Serial.print(blueMax); Serial.println(";");

// }

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

void collect_and_output_rgb_samples() {
  while (true) {
    light_LED();
    Serial.println("Enter ball colour label (or type STOP to finish):");
    // Wait for user input
    String label = "";
    while (label.length() == 0) {
      light_LED();
      if (Serial.available()) {
        light_LED();
        label = Serial.readStringUntil('\n');
        label.trim();
      }
    }
    if (label.equalsIgnoreCase("STOP")) {
      Serial.println("Data collection finished.");
      break;
    }
    //Serial.print("Ready to collect samples for: ");
    //Serial.println(label);
    //Serial.println("Press any key for each new ball of this colour (10 times recommended).");

    for (int i = 0; i < 10; ++i) {
      // Wait for key press
      while (!Serial.available());
      while (Serial.available()) Serial.read(); // clear buffer

      // Take a reading
      sample_scaled_RGB();
      // Output in CSV format: LABEL,R,G,B
      Serial.print(label); Serial.print(",");
      Serial.print(redVal); Serial.print(",");
      Serial.print(greenVal); Serial.print(",");
      Serial.println(blueVal);
      //Serial.print("Sample "); Serial.print(i+1); Serial.println(" recorded.");
    }
    //Serial.println("Done with this colour.");
  }
}