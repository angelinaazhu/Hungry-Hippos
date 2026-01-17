#pragma once
void spinRevs(float revs, int dir, AccelStepper& stepper, const long STEPS_PER_REV);
float angleDifference(float start, float now);
void spin90WithCorrection(AccelStepper& stepper, const long STEPS_PER_REV, const int ENCODER_PIN, int dir);