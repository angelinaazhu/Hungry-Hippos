#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

//return false = time not up
// return true = time up
bool mini_countdown(Adafruit_7segment& hex_timer, unsigned long& currentMillis, unsigned long& previousMillis, unsigned int& secondsUntilGo, bool& start){
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
      return false;
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
      return true;
    }
  }
}

//return false = time not up
//return true = time up
bool countdown(Adafruit_7segment& hex_timer1, Adafruit_7segment& hex_timer2, unsigned long& currentMillis, unsigned long& previousMillis, unsigned int& remainingSeconds){
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

    //PRINT ON hex_timer1
    hex_timer1.writeDigitNum(0, min10);
    hex_timer1.writeDigitNum(1, min1);
    hex_timer1.drawColon(true);
    hex_timer1.writeDigitNum(3, sec10);
    hex_timer1.writeDigitNum(4, sec1);
    hex_timer1.writeDisplay();

    //PRINT on hex_timer2
    hex_timer2.writeDigitNum(0, min10);
    hex_timer2.writeDigitNum(1, min1);
    hex_timer2.drawColon(true);
    hex_timer2.writeDigitNum(3, sec10);
    hex_timer2.writeDigitNum(4, sec1);
    hex_timer2.writeDisplay();

    if (remainingSeconds > 0) {
      --remainingSeconds;
      return false;
    } else {
      //timer1
      Serial.println("TIME'S UP!");
      hex_timer1.writeDigitAscii(0, 68);//d
      hex_timer1.writeDigitAscii(1, 79);//o
      hex_timer1.drawColon(false);
      hex_timer1.writeDigitAscii(3, 78);//n
      hex_timer1.writeDigitAscii(4, 69);//e
      hex_timer1.writeDisplay();
      //timer2
      hex_timer2.writeDigitAscii(0, 68);//d
      hex_timer2.writeDigitAscii(1, 79);//o
      hex_timer2.drawColon(false);
      hex_timer2.writeDigitAscii(3, 78);//n
      hex_timer2.writeDigitAscii(4, 69);//e
      hex_timer2.writeDisplay();
      return true;
    }
  
  }

}