#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

void mini_countdown(Adafruit_7segment& hex_timer, unsigned long& currentMillis, unsigned long& previousMillis, unsigned int& secondsUntilGo, bool& start){
  //Serial.println("mini_countdown() called");
  currentMillis = millis();

  // every 1000 ms, decrement remainingSeconds (if > 0) and print
  if (currentMillis - previousMillis >= 1000) {
    previousMillis = currentMillis;
    
    // compute minutes and seconds
    unsigned int minutes = secondsUntilGo / 60;
    unsigned int seconds = secondsUntilGo % 60;

    // split into tens/ones if you really want digits
    unsigned int min10 = minutes / 10;
    unsigned int min1  = minutes % 10;
    unsigned int sec10 = seconds / 10;
    unsigned int sec1  = seconds % 10;

    //PRINT IN SERIAL
    // print as mm:ss
    // Serial.print(min10);
    // Serial.print(min1);
    // Serial.print(':');
    // Serial.print(sec10);
    // Serial.println(sec1);

    //PRINT ON hex_timer
    hex_timer.writeDigitNum(0, min10);
    hex_timer.writeDigitNum(1, min1);
    hex_timer.drawColon(true);
    hex_timer.writeDigitNum(3, sec10);
    hex_timer.writeDigitNum(4, sec1);
    hex_timer.writeDisplay();

    if (secondsUntilGo > 0) {
      --secondsUntilGo;
    } else {
      start = true;
      Serial.println("GAME START!");
      hex_timer.writeDigitAscii(0, 71);
      hex_timer.writeDigitAscii(1, 79);
      hex_timer.drawColon(false);
      hex_timer.writeDigitAscii(3, 79);
      hex_timer.writeDigitAscii(4, 79);
      hex_timer.writeDisplay();
      previousMillis = millis();
      return;
    }
  }
}

void countdown(Adafruit_7segment& hex_timer, unsigned long& currentMillis, unsigned long& previousMillis, unsigned int& remainingSeconds){
  //Serial.println("countdown() called");
  currentMillis = millis();

  // every 1000 ms, decrement remainingSeconds (if > 0) and print
  if (currentMillis - previousMillis >= 1000) {
    previousMillis = currentMillis;
    
    // compute minutes and seconds
    unsigned int minutes = remainingSeconds / 60;
    unsigned int seconds = remainingSeconds % 60;

    // split into tens/ones if you really want digits
    unsigned int min10 = minutes / 10;
    unsigned int min1  = minutes % 10;
    unsigned int sec10 = seconds / 10;
    unsigned int sec1  = seconds % 10;

    //PRINT IN SERIAL
    // print as mm:ss
    // Serial.print(min10);
    // Serial.print(min1);
    // Serial.print(':');
    // Serial.print(sec10);
    // Serial.println(sec1);

    //PRINT ON hex_timer
    hex_timer.writeDigitNum(0, min10);
    hex_timer.writeDigitNum(1, min1);
    hex_timer.drawColon(true);
    hex_timer.writeDigitNum(3, sec10);
    hex_timer.writeDigitNum(4, sec1);
    hex_timer.writeDisplay();

    if (remainingSeconds > 0) {
      --remainingSeconds;
    } else {
      Serial.println("TIME'S UP!");
      hex_timer.writeDigitAscii(0, 68);
      hex_timer.writeDigitAscii(1, 79);
      hex_timer.drawColon(false);
      hex_timer.writeDigitAscii(3, 78);
      hex_timer.writeDigitAscii(4, 69);
      hex_timer.writeDisplay();
      while (1) {
        //Serial.println("Stay here until game reset (TO IMPLEMENT)");
      }
    }
  
  }

}