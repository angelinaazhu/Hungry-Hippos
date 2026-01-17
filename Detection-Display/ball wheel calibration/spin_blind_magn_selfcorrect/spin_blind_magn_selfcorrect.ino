/*
this makes the stepper motor spin blind what it thinks 90 degrees is,
then the encoder checks if it's actually 90 degrees +/-5 degrees and makes the motor self correct
*/

#include <AccelStepper.h>

const int STEP_PIN = 8;
const int DIR_PIN = 9;
const int ENCODER_PIN = A0;
const float RUN_SPEED = 1000;      // steps per second for blind spin
const float CORRECT_SPEED = 1000;  // steps per second for corrections
const float TARGET_DELTA = 90.0;   // target rotation per blind spin
const float TOLERANCE = 5.0;       // +/- degrees acceptable
const float MAXSPEED = 1000;
const float MAXACCEL = 1000;

AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

// =========================
// Helper: angle delta with wrap-around
// =========================
float angleDifference(float start, float now) {
  float d = now - start;
  if (d < -180) d += 360;
  if (d > 180) d -= 360;
  return d;
}

// Read encoder and convert to angle
float readAngle() {
  int raw = analogRead(ENCODER_PIN);
  return raw * (360.0 / 1023.0);
}

// Spin a relative number of degrees (positive or negative) using encoder feedback
void spinWithCorrection(float degrees) {
  // 1️⃣ Read start angle
  float startAngle = readAngle();

  // 2️⃣ Blindly spin stepper by requested degrees (open loop)
  // Convert degrees to approximate stepper steps (optional: if you know steps per rev)
  stepper.setSpeed(RUN_SPEED);
  float blindTargetAngle = startAngle + degrees;

  // Keep spinning until roughly 90° rotation in open loop
  while (true) {
    stepper.runSpeed();
    float nowAngle = readAngle();
    float delta = angleDifference(startAngle, nowAngle);

    if (abs(delta) >= degrees) {
      break; // completed blind spin
    }
  }

  stepper.setSpeed(0); // stop blind spin

  // 3️⃣ Self-correction using encoder
  while (true) {
    float nowAngle = readAngle();
    float delta = angleDifference(startAngle, nowAngle);
    float error = degrees - delta; // how many degrees we are off

    // Check if within tolerance
    if (abs(error) <= TOLERANCE) {
      break; // done
    }

    // Spin forward or backward to correct
    stepper.setSpeed((error > 0 ? 1 : -1) * CORRECT_SPEED);
    while (true) {
      stepper.runSpeed();
      float newDelta = angleDifference(startAngle, readAngle());
      float newError = degrees - newDelta;
      if ((error > 0 && newDelta >= degrees) || (error < 0 && newDelta <= degrees)) {
        stepper.setSpeed(0);
        break;
      }
    }
  }

  stepper.setSpeed(0);
  Serial.print("Final delta = ");
  Serial.println(angleDifference(startAngle, readAngle()));
  Serial.println("90° spin completed with correction.");
}

void setup() {
  Serial.begin(9600);
  stepper.setMaxSpeed(MAXSPEED);
  stepper.setAcceleration(MAXACCEL);
  delay(500);
}

void loop() {
  spinWithCorrection(TARGET_DELTA);
  delay(2000); // wait before next 90° spin
}
