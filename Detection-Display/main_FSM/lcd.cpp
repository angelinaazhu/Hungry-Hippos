#include <Arduino.h>
#include <Wire.h>
#include <stdlib.h>
#include "rgb_lcd.h"

bool startingMessage(rgb_lcd& lcd) {
  //Serial.println("startingMessage called");
  //TCASel(player1);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Game Starting in:");

  // lcd.setCursor(0, 1);
  // lcd.print("3");

  // delay(1000);
  // lcd.clear();

  // lcd.setCursor(0, 0);
  // lcd.print("Game Starting in:");

  // lcd.setCursor(0, 1);
  // lcd.print("2");

  // delay(1000);
  // lcd.clear();

  // lcd.setCursor(0, 0);
  // lcd.print("Game Starting in:");

  // lcd.setCursor(0, 1);
  // lcd.print("1");

  // delay(1000);
  // lcd.clear();

  // lcd.setCursor(0, 0);
  // lcd.print("GO!");

  return true;
  
}


void gameScreen(rgb_lcd& lcd){
  lcd.clear();
  lcd.setCursor(0, 0);
  //delay(500);

  lcd.setCursor(0,1);
  lcd.print("TIME");

  // lcd.setCursor(8,0);
  // lcd.print("|");
  lcd.setCursor(8,1);
  lcd.print("|");

  lcd.setCursor(10,1);
  lcd.print("POINTS");

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

void win(rgb_lcd& lcd, int n) {
  //Serial.println("win called");
  lcd.clear();
  lcd.setRGB(10, 255, 10);
  lcd.print("You won!");
  lcd.setCursor(0,2);
  lcd.print("Score: ");
  lcd.setCursor(7,2);
  lcd.print(n);
}

void loss(rgb_lcd& lcd, int n) {
  //Serial.println("loss called");
  lcd.clear();
  lcd.setRGB(255, 10, 10);
  lcd.print("You lost!");
  lcd.setCursor(0,2);
  lcd.print("Score: ");
  lcd.setCursor(7,2);
  lcd.print(n);
}

void tie(rgb_lcd& lcd, int n) {
  //Serial.println("tie called");
  lcd.clear();
  lcd.setRGB(10, 10, 255);
  lcd.print("Tie!");
  lcd.setCursor(0,2);
  lcd.print("Score: ");
  lcd.setCursor(7,2);
  lcd.print(n);
}