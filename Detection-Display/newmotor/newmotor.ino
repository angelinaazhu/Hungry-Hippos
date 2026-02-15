#include <AccelStepper.h>

// AccelStepper(step interface, STEP pin, DIR pin)
AccelStepper stepper(AccelStepper::DRIVER, 8, 9);

void setup() {
  stepper.setMaxSpeed(1000);     // steps per second (safe start)
  stepper.setSpeed(400);         // constant speed
  stepper.setMinPulseWidth(5);   // REQUIRED for TB6600-style drivers
}

void loop() {
  stepper.runSpeed();            // motor runs continuously
}
