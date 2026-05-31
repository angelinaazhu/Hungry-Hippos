#include <Servo.h>
#include <Arduino.h>

void spinStart(Servo& servo){
  servo.write(80);
  //delay(1550);
  delay(1450);
  servo.write(90);
}

void spinBack(Servo& servo){
  servo.write(100);
  //delay(1625);
  delay(1650);
  servo.write(90);
}