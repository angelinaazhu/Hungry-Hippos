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