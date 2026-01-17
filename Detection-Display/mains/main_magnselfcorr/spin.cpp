#include <AccelStepper.h>

void spinRevs(float revs, int dir, AccelStepper& stepper,
              const long STEPS_PER_REV) {
  // Serial.println("spin start");
  long steps = (long)(revs * STEPS_PER_REV +
                      0.5f);  // rounds to nearest int, then converts to long
                              // 1/4 rev * 12800 steps per rev = 3200 steps
  stepper.moveTo(
      stepper.currentPosition() +
      dir * steps);  // declares that motor will move 3200 steps fwd/bwd
                     // stepper.currentPosition() is the step motor is at
                     // moveTo() is new absolute target position
                     // dir +1 CW = adds steps
                     // dir -1 CCW = subtracts steps
  while (stepper.distanceToGo() != 0) {
    // distanceToGo() is num steps left to reach target
    // as long as there are still steps left, stay in this loop
    stepper.run();  // stepper.run() executes motor steps one at a time
  }  // exits when motor done spinning the 1/4th rev
  // Serial.println("spin stop");
}

float angleDifference(float start, float now) {
  float d = now - start;  // difference between now angle and start angle
  if (d < -180)
    d += 360;  // if difference is smaller than -180 then it adds 360 to get the
               // shortest distance
  if (d > 180) d -= 360;  //
  return d;               // d is [-180, 180], neg = spun CCW, pos = spun CW
}

// before dinner: this version overcorrects
void spin90WithCorrection(AccelStepper& stepper, const long STEPS_PER_REV,
                          const int ENCODER_PIN, int dir) {
  const float TARGET_DELTA = 90.0;  // target spin in degrees
  const float TOLERANCE = 5.0;      // degrees tolerance

  // --- Read starting angle ---
  float startRaw = analogRead(ENCODER_PIN);
  float startAngle =
      startRaw * (360.0 / 1023.0);  // Map raw value to 0-360 degrees
  Serial.print("Start angle = ");
  Serial.println(startAngle);

  Serial.println("Blind 90 deg spin FIRST");
  // --- Blind 90° spin in chosen direction ---
  spinRevs(1.0 / 4.0f, dir, stepper,
           STEPS_PER_REV);  // dir = 1 for CW, -1 for CCW
  delay(500);               // Allow motor to settle

  // --- Corrective spins ---
  while (true) {
    // Read the current angle
    float currentRaw = analogRead(ENCODER_PIN);
    float currentAngle =
        currentRaw * (360.0 / 1023.0);  // Map raw value to 0-360 degrees

    // Calculate the angle difference
    float delta = angleDifference(startAngle, currentAngle);
    Serial.print("Current angle = ");
    Serial.println(currentAngle);
    Serial.print("Delta = ");
    Serial.println(delta);

    // Check if the angle is within tolerance
    if (fabs(delta - TARGET_DELTA) <= TOLERANCE) {
      Serial.println("Spin reached target within tolerance!");
      break;
    }

    // Calculate the correction needed
    float correction =
        (delta - TARGET_DELTA) / 360.0;  // Fraction of a full revolution
    Serial.print("Correction needed (revs): ");
    Serial.println(correction);

    // Apply the correction
    spinRevs(correction, dir, stepper, STEPS_PER_REV);
    delay(500);  // Allow motor to settle after correction
  }
}
