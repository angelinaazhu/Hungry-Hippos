#pragma once
void spinRevs(float revs, int dir, AccelStepper& stepper, const long STEPS_PER_REV);
// float angleDifference(float start, float now);
// void spinMagn(const int ENCODER_PIN, const int RUN_SPEED, const int TARGET_DELTA, AccelStepper& stepper, const int DIR);

void checkMagnetPresence();
bool as5600Connected();

uint8_t readMagnetStatus();
bool isMagnetOK();

float getAbsAngle();
float getRelativeAngle(float absAngle, float startAngle);
bool checkRotated90(int direction, float startAngle, float &lastCheckpointAngle, float CHECKPOINT);
//void spinMagnI2C(const int RUN_SPEED, const int TARGET_DELTA, AccelStepper& stepper, const int DIR, const float startAngle, float &lastCheckpointAngle);
void spinBackToZero(AccelStepper& stepper, const int DIR, const float TOLERANCE, const int RUN_SPEED);