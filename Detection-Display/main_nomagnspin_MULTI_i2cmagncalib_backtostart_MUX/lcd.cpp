#include <Arduino.h>
#include <Wire.h>
#include <stdlib.h>
#include "rgb_lcd.h"

void startingMessage(rgb_lcd& lcd) {
  Serial.println("startingMessage called");
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Game Starting in:");

  lcd.setCursor(0, 1);
  lcd.print("3");

  delay(1000);
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Game Starting in:");

  lcd.setCursor(0, 1);
  lcd.print("2");

  delay(1000);
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Game Starting in:");

  lcd.setCursor(0, 1);
  lcd.print("1");

  delay(1000);
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("GO!");

  delay(1000);
  
}