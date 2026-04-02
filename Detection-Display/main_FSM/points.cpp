#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

void display_points(int points, Adafruit_7segment& hex_points){
  int final_score = points;
  int thousandths;
  int hundredths;
  int tens;
  int ones;

  ones = final_score % 10;
  Serial.println(ones);
  hex_points.writeDigitNum(4,ones);

  tens = final_score % 100;
  tens = tens /10;
  Serial.println(tens);
  hex_points.writeDigitNum(3,tens);

  hundredths = final_score % 1000;
  hundredths = hundredths /100;
  Serial.println(hundredths);
  hex_points.writeDigitNum(1,hundredths);

  thousandths = final_score % 10000;
  thousandths = thousandths /1000;
  Serial.println(thousandths);
  hex_points.writeDigitNum(0,thousandths);

  hex_points.writeDisplay();

}