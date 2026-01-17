#include <AccelStepper.h>

// =========================
// AS5600 analog setup
// =========================
#define ENCODER_PIN A0

// int readEncoderRaw() {
//   return analogRead(ENCODER_PIN);
// }

// float rawToDegrees(int raw) {
//   return raw * (360.0 / 1023.0);
// }

// =========================
// Motor setup
// =========================
const int STEP_PIN   = 8;
const int DIR_PIN    = 9;
const int ENABLE_PIN = -1;

AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

const float RUN_SPEED = 2000;    // steps per second (choose what works)
const float TARGET_DELTA = 90.0; // 1/4 revolution

// =========================
// Helper: angle delta with wrap-around
// =========================
float angleDifference(float start, float now) {
  float d = now - start;
  if (d < -180) d += 360;
  if (d > 180)  d -= 360;
  return d;
}

void setup() {
  Serial.begin(9600);
  stepper.setMaxSpeed(10000000000000000000);   // higher than run speed
  stepper.setAcceleration(10000000000000000000);
  delay(500);
}

void loop() {
  // ---- Read starting angle ----
  float startAngle = (analogRead(ENCODER_PIN))*(360.0 / 1023.0);
  Serial.print("Start angle = ");
  Serial.println(startAngle);

  // ---- Start rotating CW ----
  stepper.setSpeed(+RUN_SPEED);

  Serial.println("Rotating 1/4 turn...");

  while (true) {
    stepper.runSpeed();  // continuous rotation

    float nowAngle = (analogRead(ENCODER_PIN))*(360.0 / 1023.0);
    float delta = angleDifference(startAngle, nowAngle);

    // Debug print occasionally
    // Serial.println(delta);

    if (delta >= TARGET_DELTA) {
      break;  // reached 1/4 revolution
    }
  }

  // ---- Stop motor ----
  stepper.setSpeed(0);
  Serial.println("Reached 1/4 turn. Stopping.");
  delay(2000);   // Pause before repeating
}