#include <AccelStepper.h>

void spinRevs(float revs, int dir, AccelStepper& stepper, const long STEPS_PER_REV){
  //Serial.println("spin start");
  long steps = (long)(revs * STEPS_PER_REV + 0.5f); // rounds to nearest int, then converts to long
                            // 1/4 rev * 12800 steps per rev = 3200 steps
  stepper.moveTo(stepper.currentPosition() + dir * steps); // declares that motor will move 3200 steps fwd/bwd
                            // stepper.currentPosition() is the step motor is at
                            // moveTo() is new absolute target position
                            // dir +1 CW = adds steps
                            // dir -1 CCW = subtracts steps
  while (stepper.distanceToGo() != 0) {
        // distanceToGo() is num steps left to reach target
        // as long as there are still steps left, stay in this loop
    stepper.run(); // stepper.run() executes motor steps one at a time
  } // exits when motor done spinning the 1/4th rev
  //Serial.println("spin stop");
}

float angleDifference(float start, float now) {
  float d = now - start;
  if (d < -180) d += 360;
  if (d > 180)  d -= 360;
  return d;
}

// void spinMagn(const int ENCODER_PIN, const int RUN_SPEED, const int TARGET_DELTA, AccelStepper& stepper, const int DIR) {
//   float startAngle = (analogRead(ENCODER_PIN))*(360.0 / 1023.0);
//   Serial.print("Start angle = ");
//   Serial.println(startAngle);

//   // ---- Start rotating CW ----
//   stepper.setSpeed(DIR*RUN_SPEED);

//   Serial.println("Rotating 1/4 turn...");

//   while (true) {
//     stepper.runSpeed();  // continuous rotation

//     float nowAngle = (analogRead(ENCODER_PIN))*(360.0 / 1023.0);
//     float delta = angleDifference(startAngle, nowAngle);

//     // Debug print occasionally
//     // Serial.println(delta);

//     if (delta >= TARGET_DELTA) {
//       break;  // reached 1/4 revolution
//     }
//   }

//   // ---- Stop motor ----
//   stepper.setSpeed(0);
//   Serial.println("Reached 1/4 turn. Stopping.");
// }
void spinMagn(const int ENCODER_PIN, const int RUN_SPEED, const int TARGET_DELTA,
              AccelStepper& stepper, const int DIR) {

  float startRaw   = analogRead(ENCODER_PIN);
  float startAngle = startRaw * (360.0 / 1023.0);

  Serial.print("Start angle = ");
  Serial.print(startAngle);
  Serial.print("  (raw=");
  Serial.print(startRaw);
  Serial.println(")");

  stepper.setSpeed(DIR * RUN_SPEED);
  Serial.println("Rotating 1/4 turn...");

  unsigned long lastPrint = 0; 
  const unsigned long PRINT_INTERVAL = 1000; // ms

  while (true) {
    stepper.runSpeed();   // keep spinning full speed

    float raw      = analogRead(ENCODER_PIN);
    float nowAngle = raw * (360.0 / 1023.0);
    float delta    = angleDifference(startAngle, nowAngle);

    // Print every 20 ms (doesn't slow motor)
    unsigned long now = millis();
    // if (now - lastPrint >= PRINT_INTERVAL) {
    //   lastPrint = now;

    //   Serial.print("raw=");
    //   Serial.print(raw);
    //   Serial.print("  angle=");
    //   Serial.print(nowAngle);
    //   Serial.print("  delta=");
    //   Serial.println(delta);
    // }

    if (delta >= TARGET_DELTA) {
      Serial.print("   delta=");
      Serial.println(delta);
      break;
    }
  }

  stepper.setSpeed(0);
  
  Serial.println("Reached 1/4 turn. Stopping.");
}

