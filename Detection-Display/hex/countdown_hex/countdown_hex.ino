#include <Wire.h> // Enable this line if using Arduino Uno, Mega, etc.
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

// #include "Wire.h"
// #define MUX_ADDR 0x70 //TCA9548A encoder address

// //helper function to help select port
// // use by doing TCAsel(0) -> TCAsel(7)
// void TCAsel(uint8_t i2c_bus){
//   if (i2c_bus > 7){
//     return;
//   }

//   Wire.beginTransmission (MUX_ADDR);
//   Wire.write(1 << i2c_bus);
//   Wire.endTransmission();
// }

Adafruit_7segment hex_timer = Adafruit_7segment();

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

 // TCAsel(2);

  hex_timer.begin(0x73);
}

void loop() {
  if (start == false){
    mini_countdown();
  } else {
    countdown();
  }
}

void initial_display(){
  hex_timer.writeDigitNum(0, 8);
  hex_timer.writeDigitNum(1, 8);
  hex_timer.drawColon(false);
  hex_timer.writeDigitNum(3, 8);
  hex_timer.writeDigitNum(4, 8);
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
      while (1) {}
    }
  
  }

}