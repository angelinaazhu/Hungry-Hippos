#include <Wire.h> // Enable this line if using Arduino Uno, Mega, etc.
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

Adafruit_7segment hex = Adafruit_7segment();

//actual countdown
unsigned long previousMillis = 0;
const unsigned int startSeconds = 10;  // 2 minutes
unsigned int remainingSeconds = startSeconds;

//mini pre game countdown
const unsigned int miniStartSeconds = 3; // 3, 2, 1, GO
unsigned int secondsUntilGo = miniStartSeconds;

unsigned int minutes, seconds;
int min10, min1, sec10, sec1;

unsigned long currentMillis;

bool start = false;

void initial_display();
void mini_countdown();
void countdown();

void setup() {
  delay(2000); // 1000 makes it glitch a little at 3, 2000 makes it glitch at prev -> probably doesnt matter because there's going to be a startup time
  Serial.begin(9600);
  hex.begin(0x70);
  hex.writeDisplay();
}

void loop() {
  if (start == false){
    mini_countdown();
  } else {
    countdown();
  }
}

void initial_display(){
  hex.writeDigitNum(0, 8);
  hex.writeDigitNum(1, 8);
  hex.drawColon(false);
  hex.writeDigitNum(3, 8);
  hex.writeDigitNum(4, 8);
}

void mini_countdown(){
  //Serial.println("go() called");
  currentMillis = millis();

  // every 1000 ms, decrement remainingSeconds (if > 0) and print
  if (currentMillis - previousMillis >= 1000) {
    previousMillis = currentMillis;
    
    // compute minutes and seconds
    minutes = secondsUntilGo / 60;
    seconds = secondsUntilGo % 60;

    // split into tens/ones if you really want digits
    min10 = minutes / 10;
    min1  = minutes % 10;
    sec10 = seconds / 10;
    sec1  = seconds % 10;

    //PRINT IN SERIAL
    // print as mm:ss
    Serial.print(min10);
    Serial.print(min1);
    Serial.print(':');
    Serial.print(sec10);
    Serial.println(sec1);

    //PRINT ON HEX
    hex.writeDigitNum(0, min10);
    hex.writeDigitNum(1, min1);
    hex.drawColon(true);
    hex.writeDigitNum(3, sec10);
    hex.writeDigitNum(4, sec1);
    hex.writeDisplay();

    if (secondsUntilGo > 0) {
      --secondsUntilGo;
    } else {
      start = true;
      Serial.println("GAME START!");
      hex.writeDigitAscii(0, 71);
      hex.writeDigitAscii(1, 79);
      hex.drawColon(false);
      hex.writeDigitAscii(3, 79);
      hex.writeDigitAscii(4, 79);
      hex.writeDisplay();
      delay(1000);
      previousMillis = millis();
      return;
    }
  }
}

void countdown(){
  //Serial.println("countdown() called");
  currentMillis = millis();

  // every 1000 ms, decrement remainingSeconds (if > 0) and print
  if (currentMillis - previousMillis >= 1000) {
    previousMillis = currentMillis;
    
    // compute minutes and seconds
    minutes = remainingSeconds / 60;
    seconds = remainingSeconds % 60;

    // split into tens/ones if you really want digits
    min10 = minutes / 10;
    min1  = minutes % 10;
    sec10 = seconds / 10;
    sec1  = seconds % 10;

    //PRINT IN SERIAL
    // print as mm:ss
    Serial.print(min10);
    Serial.print(min1);
    Serial.print(':');
    Serial.print(sec10);
    Serial.println(sec1);

    //PRINT ON HEX
    hex.writeDigitNum(0, min10);
    hex.writeDigitNum(1, min1);
    hex.drawColon(true);
    hex.writeDigitNum(3, sec10);
    hex.writeDigitNum(4, sec1);
    hex.writeDisplay();

    if (remainingSeconds > 0) {
      --remainingSeconds;
    } else {
      Serial.println("TIME'S UP!");
      hex.writeDigitAscii(0, 68);
      hex.writeDigitAscii(1, 79);
      hex.drawColon(false);
      hex.writeDigitAscii(3, 78);
      hex.writeDigitAscii(4, 69);
      hex.writeDisplay();
      while (1) {}
    }
  
  }

}