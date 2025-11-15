#include <AccelStepper.h>

// Arduino I/O pins
const int STEP_PIN   = 8;
const int DIR_PIN    = 9;
const int ENABLE_PIN = -1; // set to your EN pin, or -1 if none

// Define stepper motor interface type and pins
AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

unsigned long startTime = 0;
const unsigned long runDuration = 20000; // 20 seconds in ms
bool motorRunning = true;

void setup() {
  stepper.setMaxSpeed(7000);  // Set max speed (steps per second)
  stepper.setSpeed(-7000);     // Set continuous speed (positive = CW, negative = CCW)

  startTime = millis();       // Record when we started
}

void loop() {
  if (motorRunning) {
    if (millis() - startTime < runDuration) {
      stepper.runSpeed();     // Run motor
    } else {
      motorRunning = false;
      stepper.stop();         // Stop smoothly (optional)
      // If using an enable pin, disable motor here
      // digitalWrite(ENABLE_PIN, HIGH);
    }
  }
  // Motor is stopped — do nothing
}
