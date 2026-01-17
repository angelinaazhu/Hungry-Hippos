/*
this makes the stepper motor spin continuously, while outputting the encoder values (angle, volts, raw adc value)
WHEN it reaches 90 degree delta angle from START -> NOW, it stops for a few seconds.
*/

#include <AccelStepper.h>

const int STEP_PIN = 8;
const int DIR_PIN = 9;
const int ENABLE_PIN = -1;
const int ENCODER_PIN = A0;
const float RUN_SPEED = 500;     // steps per second (choose what works)
const float TARGET_DELTA = 90.0;  // 1/4 revolution
const float MAXSPEED = 500;
const float MAXACCEL = 500;

AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

// =========================
// Helper: angle delta with wrap-around: 0->360
// =========================
float angleDifference(float start, float now) {
  float d = now - start;
  if (d < -180) d += 360;
  if (d > 180) d -= 360;
  return d;
}


void spinMagn(const int ENCODER_PIN, const int RUN_SPEED, const int TARGET_DELTA, AccelStepper& stepper) {
  //read raw ADC 0->1023 from magnetic encoder & convert to angle
  //raw value ADC * 360 degrees/1023 ADC -> want to make value from
  // out of 1023 -> out of 360
  float startraw = analogRead(ENCODER_PIN);
  float startvolt = startraw * (5.0/1023.0);
  float startangle = startraw * (360.0 / 1023.0);

  Serial.println();
  Serial.print("START raw = ");
  Serial.print(startraw);
  Serial.print(" | volt = ");
  Serial.print(startvolt);
  Serial.print(" | angle = ");
  Serial.println(startangle);

  // ---- Start rotating CW ----
  stepper.setSpeed(+RUN_SPEED);

  Serial.println("Rotating 1/4 turn...");

  unsigned long lastPrint = 0;

  while (true) {
    stepper.runSpeed();  // continuous rotation
    float nowraw = analogRead(ENCODER_PIN);
    float nowvolt = nowraw * (5.0/1023.0);
    float nowangle = nowraw * (360.0 / 1023.0);
    float delta = angleDifference(startangle, nowangle);

    if (millis() - lastPrint > 500) {
      lastPrint = millis();
      Serial.print("NOW ");
      //Serial.print("raw = ");
      //Serial.println(nowraw);
      //Serial.print(" | volt = ");
      //Serial.print(nowvolt);
      //Serial.print(" | angle = ");
      //Serial.println(nowangle);
      Serial.print("angle diff = ");
      Serial.println(delta);
    }

    // Debug print occasionally
    // Serial.println(delta);

    if (delta >= TARGET_DELTA) {
      Serial.print("STOPPING angle diff = ");
      Serial.println(delta);
      break;  // reached 1/4 revolution
    }
  }

  // ---- Stop motor ----
  stepper.setSpeed(0);
  Serial.println("Reached 1/4 turn. Stopping.");
}

void setup() {
  Serial.begin(9600);
  stepper.setMaxSpeed(MAXSPEED);  // higher than run speed
  stepper.setAcceleration(MAXACCEL);
  delay(500);
}

void loop() {
  spinMagn(ENCODER_PIN, RUN_SPEED, TARGET_DELTA, stepper);
  delay(2000);  // Pause before repeating
}
