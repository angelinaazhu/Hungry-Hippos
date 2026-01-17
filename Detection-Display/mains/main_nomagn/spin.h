#pragma once
void spinRevs(float revs, int dir, AccelStepper& stepper, const long STEPS_PER_REV);
float angleDifference(float start, float now);
void spinMagn(const int ENCODER_PIN, const int RUN_SPEED, const int TARGET_DELTA, AccelStepper& stepper);