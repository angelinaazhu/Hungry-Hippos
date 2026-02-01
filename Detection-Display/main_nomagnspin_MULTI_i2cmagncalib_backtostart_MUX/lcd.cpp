#include <Arduino.h>
#include <Wire.h>
#include <stdlib.h>
#include "rgb_lcd.h"

bool startingMessage(rgb_lcd& lcd) {
  //Serial.println("startingMessage called");
  
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

  return true;
  
}


void gameScreen(rgb_lcd& lcd){
  lcd.clear();
  lcd.setCursor(0, 0);
  //delay(500);

  lcd.setCursor(0,1);
  lcd.print("Points");

  // lcd.setCursor(8,0);
  // lcd.print("|");
  lcd.setCursor(8,1);
  lcd.print("|");

  lcd.setCursor(12,1);
  lcd.print("Time");

  // lcd.setCursor(0,1);
  // lcd.print("V"); 

  // lcd.setCursor(15,1);
  // lcd.print("V");

}

void coolDownMsg(rgb_lcd& lcd) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Counting Points");

  lcd.setCursor(8, 1);
  lcd.print("...");
}