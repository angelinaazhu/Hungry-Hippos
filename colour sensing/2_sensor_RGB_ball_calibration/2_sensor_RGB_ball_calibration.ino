#include <Wire.h>
#include "SFE_ISL29125.h"

/************LED vars***************/
#include <Adafruit_NeoPixel.h>
#define LED_PIN 6 // Pin where DIN (pin 4 of WS2812D-265) is connected
#define NUM_LEDS 2 // Number of LEDs in your strip (or single LED)
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void light_LED(){
  strip.begin();        // Initialize LED
  strip.show();         // Turn off all LEDs initially
  strip.setBrightness(255); // Max brightness (0-255)
  strip.setPixelColor(0, strip.Color(255, 255, 255));
  strip.setPixelColor(1, strip.Color(255, 255, 255));
  strip.show();         // Update LED
}
/********END OF LED vars************/

SFE_ISL29125 RGB_sensor;

// Your scaling bounds (from your last run)
const unsigned int redLow = 44;
const unsigned int redHigh = 1362;
const unsigned int greenLow = 56;
const unsigned int greenHigh = 2635;
const unsigned int blueLow = 33;
const unsigned int blueHigh = 1240; 

// calibrated averages
float envAvgR, envAvgG, envAvgB;
float blueBallAvgR, blueBallAvgG, blueBallAvgB;
float yellowBallAvgR, yellowBallAvgG, yellowBallAvgB;
float redBallAvgR, redBallAvgG, redBallAvgB;

// temporary scaled readings
int redVal, greenVal, blueVal;
int redScaled, greenScaled, blueScaled;

// timing
const unsigned long CALIB_DURATION  = 10000UL;  // 30 seconds
const unsigned long SAMPLE_INTERVAL = 1000UL;   // 1 s between samples

// helper to read & scale into [0–255]
void sampleScaledRGB() {
  unsigned int redRaw = RGB_sensor.readRed();
  unsigned int greenRaw = RGB_sensor.readGreen();
  unsigned int blueRaw = RGB_sensor.readBlue();

  // skip any 0,0,0 startup sample
  if (redRaw == 0 && greenRaw == 0 && blueRaw == 0) {
    delay(1000);
    return;
  }

  redScaled = map(redRaw, redLow, redHigh, 0, 255);
  greenScaled = map(greenRaw, greenLow, greenHigh, 0, 255);
  blueScaled = map(blueRaw, blueLow, blueHigh, 0, 255);

  redVal = constrain(redScaled, 0, 255);
  greenVal = constrain(greenScaled, 0, 255);
  blueVal = constrain(blueScaled, 0, 255);
}

void calibration(){
  // --- 1) Environment calibration ---
  Serial.println(" ");
  Serial.println("=== ENVIRONMENT CALIBRATION ===");
  Serial.println("Ensure NO ball is visible. Calibrating...");

  unsigned long start = millis();
  unsigned long count = 0;
  unsigned long sumR = 0, sumG = 0, sumB = 0; // accumulate all R, G, B values

  while (millis() - start < CALIB_DURATION) { //while curr time - start < 30s
                                              // = while time passed < 30s
    light_LED();
    sampleScaledRGB(); // red/green/blueVals all have scaled 225 vals
    sumR += redVal; // accumulate
    sumG += greenVal;
    sumB += blueVal;
    count++;
    Serial.print(".");  // progress indicator
    delay(SAMPLE_INTERVAL);
  }

  envAvgR = float(sumR) / count; //avg of R: totalR / num of accumulations
  envAvgG = float(sumG) / count;
  envAvgB = float(sumB) / count;

  Serial.println();
  Serial.print("Env avg R,G,B = ");
  Serial.print(envAvgR); Serial.print(", ");
  Serial.print(envAvgG); Serial.print(", ");
  Serial.println(envAvgB);

  
  // --- 2) BLUE Ball calibration ---
  // wait for user to put ball in view
  Serial.println("\nPress any key to begin BLUE BALL calibration");
  while (!Serial.available());
  Serial.read();  // clear the incoming byte

  Serial.println("=== BLUE BALL CALIBRATION ===");
  Serial.println("Place BLUE BALL in view. Calibrating...");

  start = millis();
  count = 0;
  sumR = sumG = sumB = 0;

  while (millis() - start < CALIB_DURATION) {
    light_LED();
    sampleScaledRGB();
    sumR += redVal;
    sumG += greenVal;
    sumB += blueVal;
    count++;
    Serial.print("*");
    delay(SAMPLE_INTERVAL);
  }

  blueBallAvgR = float(sumR) / count;
  blueBallAvgG = float(sumG) / count;
  blueBallAvgB = float(sumB) / count;

  Serial.println();
  Serial.print("Blue Ball avg R,G,B = ");
  Serial.print(blueBallAvgR); Serial.print(F(", "));
  Serial.print(blueBallAvgG); Serial.print(F(", "));
  Serial.println(blueBallAvgB);
  
  
  // --- 3) YELLOW Ball calibration ---
  // …after finishing BLUE calibration…
  while (Serial.available()) Serial.read(); 
  // wait for user to put ball in view
  Serial.println("\nPress any key to begin YELLOW BALL calibration");
  while (!Serial.available());
  Serial.read();  // clear the incoming byte

  Serial.println("=== YELLOW BALL CALIBRATION ===");
  Serial.println("Place YELLOW BALL in view. Calibrating...");

  start = millis();
  count = 0;
  sumR = sumG = sumB = 0;

  while (millis() - start < CALIB_DURATION) {
    light_LED();
    sampleScaledRGB();
    sumR += redVal;
    sumG += greenVal;
    sumB += blueVal;
    count++;
    Serial.print("*");
    delay(SAMPLE_INTERVAL);
  }

  yellowBallAvgR = float(sumR) / count;
  yellowBallAvgG = float(sumG) / count;
  yellowBallAvgB = float(sumB) / count;

  Serial.println();
  Serial.print("Yellow Ball avg R,G,B = ");
  Serial.print(yellowBallAvgR); Serial.print(F(", "));
  Serial.print(yellowBallAvgG); Serial.print(F(", "));
  Serial.println(yellowBallAvgB);
  
  // --- 4) RED Ball calibration ---
  // …after finishing BLUE calibration…
  //COMMENTED OUT RED CUZ I DONT HAVE RED BALL HERE
  /*while (Serial.available()) Serial.read(); 
  // wait for user to put ball in view
  Serial.println("\nPress any key to begin RED BALL calibration");
  while (!Serial.available());
  Serial.read();  // clear the incoming byte

  Serial.println("=== RED BALL CALIBRATION ===");
  Serial.println("Place RED BALL in view. Calibrating...");

  start = millis();
  count = 0;
  sumR = sumG = sumB = 0;

  while (millis() - start < CALIB_DURATION) {
    light_LED();
    sampleScaledRGB();
    sumR += redVal;
    sumG += greenVal;
    sumB += blueVal;
    count++;
    Serial.print("*");
    delay(SAMPLE_INTERVAL);
  }

  redBallAvgR = float(sumR) / count;
  redBallAvgG = float(sumG) / count;
  redBallAvgB = float(sumB) / count;

  Serial.println();
  Serial.print("Red Ball avg R,G,B = ");
  Serial.print(redBallAvgR); Serial.print(F(", "));
  Serial.print(redBallAvgG); Serial.print(F(", "));
  Serial.println(redBallAvgB);*/
  Serial.println("\n*** Calibration complete — entering detection loop ***");
}

void setup() {
  Serial.begin(9600);

  if (!RGB_sensor.init()) {
    Serial.println("Sensor init failed!");
    while (1);
  }
  
  light_LED();

  // discard first conversion -- probably 0
  RGB_sensor.readRed();
  RGB_sensor.readGreen();
  RGB_sensor.readBlue();
  delay(100);
  
  calibration();

}

void loop() {
  light_LED();
  Serial.println(F("\nPress any key to begin BALL classification"));
  while (!Serial.available()) {
    ; // do nothing until a key comes in
  }
  Serial.read();               // clear the incoming byte
  while (Serial.available())   // flush any extra bytes
    Serial.read();

  const unsigned long WINDOW_MS   = 1000UL;
  const unsigned long INTERVAL_MS = 100UL;   // sample every 100 ms
  unsigned long start = millis();

  // counts[0] = no ball, [1]=blue, [2]=yellow, [3]=red
  unsigned int counts[4] = { 0, 0, 0, 0 };

  while (millis() - start < WINDOW_MS) {
    sampleScaledRGB();  

    // compute all 4 distances
    float distEnv    = sq(redVal - envAvgR)    + sq(greenVal - envAvgG)    + sq(blueVal - envAvgB);
    float distBlue   = sq(redVal - blueBallAvgR)   + sq(greenVal - blueBallAvgG)   + sq(blueVal - blueBallAvgB);
    float distYellow = sq(redVal - yellowBallAvgR) + sq(greenVal - yellowBallAvgG) + sq(blueVal - yellowBallAvgB);
    //float distRed    = sq(redVal - redBallAvgR)    + sq(greenVal - redBallAvgG)    + sq(blueVal - redBallAvgB); //COMMENTED OUT RED CUZ I DONT HAVE RED BALL HERE

    // pick the smallest
    float minDist = distEnv;
    int   choice  = 0;  // 0=no, 1=blue, 2=yellow, 3=red
    if (distBlue   < minDist) { minDist = distBlue;   choice = 1; }
    if (distYellow < minDist) { minDist = distYellow; choice = 2; }
    //if (distRed    < minDist) { minDist = distRed;    choice = 3; } //COMMENTED OUT RED CUZ I DONT HAVE RED BALL HERE

    counts[choice]++;      // tally this vote
    delay(INTERVAL_MS);
  }

  // 3) pick the colour with the most votes
  int best = 0;
  for (int i = 1; i < 4; i++) {
    if (counts[i] > counts[best]) best = i;
  }

  // 4) print just one final result
  switch (best) {
    case 0: Serial.println(F(">>> NO BALL"));     break;
    case 1: Serial.println(F(">>> BLUE BALL"));   break;
    case 2: Serial.println(F(">>> YELLOW BALL")); break;
    case 3: Serial.println(F(">>> RED BALL"));    break;
  }

  // then loop() repeats, waiting for the next Enter‑key press
}