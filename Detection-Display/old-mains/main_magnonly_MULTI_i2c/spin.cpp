#include <AccelStepper.h>
#include <Wire.h>

// void spinRevs(float revs, int dir, AccelStepper& stepper, const long STEPS_PER_REV){
//   //Serial.println("spin start");
//   long steps = (long)(revs * STEPS_PER_REV + 0.5f); // rounds to nearest int, then converts to long
//                             // 1/4 rev * 12800 steps per rev = 3200 steps
//   stepper.moveTo(stepper.currentPosition() + dir * steps); // declares that motor will move 3200 steps fwd/bwd
//                             // stepper.currentPosition() is the step motor is at
//                             // moveTo() is new absolute target position
//                             // dir +1 CW = adds steps
//                             // dir -1 CCW = subtracts steps
//   while (stepper.distanceToGo() != 0) {
//         // distanceToGo() is num steps left to reach target
//         // as long as there are still steps left, stay in this loop
//     stepper.run(); // stepper.run() executes motor steps one at a time
//   } // exits when motor done spinning the 1/4th rev
//   //Serial.println("spin stop");
// }

// float angleDifference(float start, float now) {
//   float d = now - start;
//   if (d < -180) d += 360;
//   if (d > 180)  d -= 360;
//   return d;
// }

// void spinMagn(const int ENCODER_PIN, const int RUN_SPEED, const int TARGET_DELTA,
//               AccelStepper& stepper, const int DIR) {

//   float startRaw   = analogRead(ENCODER_PIN);
//   float startAngle = startRaw * (360.0 / 1023.0);

//   Serial.print("Start angle = ");
//   Serial.print(startAngle);
//   Serial.print("  (raw=");
//   Serial.print(startRaw);
//   Serial.println(")");

//   stepper.setSpeed(DIR * RUN_SPEED);
//   Serial.println("Rotating 1/4 turn...");

//   unsigned long lastPrint = 0; 
//   const unsigned long PRINT_INTERVAL = 1000; // ms

//   while (true) {
//     stepper.runSpeed();   // keep spinning full speed

//     float raw      = analogRead(ENCODER_PIN);
//     float nowAngle = raw * (360.0 / 1023.0);
//     float delta    = angleDifference(startAngle, nowAngle);

//     // Print every 20 ms (doesn't slow motor)
//     unsigned long now = millis();
//     // if (now - lastPrint >= PRINT_INTERVAL) {
//     //   lastPrint = now;

//     //   Serial.print("raw=");
//     //   Serial.print(raw);
//     //   Serial.print("  angle=");
//     //   Serial.print(nowAngle);
//     //   Serial.print("  delta=");
//     //   Serial.println(delta);
//     // }

//     if (delta >= TARGET_DELTA) {
//       Serial.print("   delta=");
//       Serial.println(delta);
//       break;
//     }
//   }

//   stepper.setSpeed(0);
  
//   Serial.println("Reached 1/4 turn. Stopping.");
// }

void checkMagnetPresence(){  
  int magnetStatus = 0; //value of the status register (MD, ML, MH)
                      //tells if magnet is detected & if strength OK

  //This function runs in the setup() and it locks the MCU until the magnet is not positioned properly
  Serial.println("Looking for magnet...");
  
  while((magnetStatus & 32) != 32) //while the magnet is not adjusted to the proper distance - 32: MD = 1
  {
    magnetStatus = 0; //reset reading

    Wire.beginTransmission(0x36); //connect to the sensor
    Wire.write(0x0B); //figure 21 - register map: Status: MD ML MH
    Wire.endTransmission(); //end transmission
    Wire.requestFrom(0x36, 1); //request from the sensor

    while(Wire.available() == 0); //wait until it becomes available 
    magnetStatus = Wire.read(); //Reading the data after the request

    //Serial.print("Magnet status: ");
    //Serial.println(magnetStatus, BIN); //print it in binary so you can compare it to the table (fig 21)      
  }      
  
  //Status register output: 0 0 MD ML MH 0 0 0  
  //MH: Too strong magnet - 100111 - DEC: 39 
  //ML: Too weak magnet - 10111 - DEC: 23     
  //MD: OK magnet - 110111 - DEC: 55

  Serial.println("Magnet found!");
  delay(1000);  
}

float getAbsAngle(){ 
  int lowbyte; //raw angle 7:0
  word highbyte; //raw angle 7:0 and 11:8
  int rawAngle; //final raw angle number (0-4095)
  float absAngle; //raw angle in degrees (360/4096 * [value between 0-4095])

  //7:0 - lower 8 bits
  Wire.beginTransmission(0x36); //connect to the sensor
  Wire.write(0x0D); //figure 21 - register map: Raw angle (7:0)
  Wire.endTransmission(); //end transmission
  Wire.requestFrom(0x36, 1); //request from the sensor
  
  while(Wire.available() == 0); //wait until it becomes available 
  lowbyte = Wire.read(); //Reading the data after the request
 
  //11:8 - upper 4 bits
  Wire.beginTransmission(0x36);
  Wire.write(0x0C); //figure 21 - register map: Raw angle (11:8)
  Wire.endTransmission();
  Wire.requestFrom(0x36, 1);
  
  while(Wire.available() == 0);  
  highbyte = Wire.read();
  
  //concatenate bits together
  //4 bits have to be shifted to its proper place as we want to build a 12-bit number
  highbyte = highbyte << 8; //shifting to left
  rawAngle = highbyte | lowbyte; //int is 16 bits (as well as the word)

  //scale 0-4095 to 0-360 degrees
  absAngle = rawAngle * (360.0/4096.0); 
  
  // Serial.print("Deg angle: ");
  // Serial.println(absAngle, 2); //absolute position of the encoder within the 0-360 circle
  return absAngle;
}

float getRelativeAngle(float absAngle, float startAngle){
  //recalculate angle
  float relAngle = absAngle - startAngle; //this tares the position
  
  //if the calculated angle is negative, we need to "normalize" it
  if(relAngle < 0){
    relAngle = relAngle + 360; //correction for negative numbers (i.e. -15 becomes +345)
  }
  
  //Serial.print("Relative angle: ");
  //Serial.println(relAngle, 2); //print the corrected/tared angle  
  return relAngle;
}

//void checkRotated90(int direction, float startAngle, float &lastCheckpointAngle, const float CHECKPOINT, AccelStepper& stepper);
/*void checkRotated90(int direction, float startAngle, float& lastCheckpointAngle, const float CHECKPOINT, AccelStepper& stepper) {
  //Serial.println("enteredcheck90");
  float absAngle, relAngle;
  // direction: 1 for CW (increasing angle), -1 for CCW (decreasing angle)
  while (true) {
    stepper.runSpeed(); 
    absAngle = getAbsAngle();
    relAngle = getRelativeAngle(absAngle, startAngle);
    float delta = relAngle - lastCheckpointAngle;
    // handle wrap-around (shortest signed difference)
    if (delta > 180.0) delta -= 360.0;
    if (delta < -180.0) delta += 360.0;
    // Only trigger if rotation is at least 90° in the desired direction
    //Serial.println(delta);
    //Serial.println(direction);
    //Serial.println(CHECKPOINT);
    if (direction * delta >= CHECKPOINT) { //-1 cancels out the negative delta if CCW
      Serial.print("Rotated 90 degrees! Delta: ");
      Serial.println(delta, 2);
      lastCheckpointAngle = relAngle;

      stepper.setSpeed(0);
      Serial.println("Stopped.");
      break;
      //return true;
    }
    delay(10); // small delay to avoid busy-waiting
  }
}*/
void checkRotated90(int direction, float startAngle, float& lastCheckpointAngle,
                    const float CHECKPOINT, AccelStepper& stepper) {
  float absAngle, relAngle;

  unsigned long lastAngleRead = 0;
  const unsigned long ANGLE_PERIOD_MS = 2; // read angle every 2 ms

  while (true) {
    // Keep stepping as fast as possible
    stepper.runSpeed();

    unsigned long now = millis();
    if (now - lastAngleRead >= ANGLE_PERIOD_MS) {
      lastAngleRead = now;

      absAngle = getAbsAngle();

      float delta = lastCheckpointAngle - absAngle;
      if (delta > 180.0)  delta -= 360.0;
      if (delta < -180.0) delta += 360.0;

      if (direction * delta >= CHECKPOINT) {
        Serial.print("Rotated 90 degrees! Delta: ");
        Serial.println(delta, 2);

        lastCheckpointAngle = absAngle;
        stepper.setSpeed(0);
        break;
      }

    }
  }
}


// spinMagnI2C(RUN_SPEED, TARGET_DELTA, stepper, -1, startAngle, lastCheckpointAngle);
//spinMagnI2C(RUN_SPEED, TARGET_DELTA, stepper, -1, startAngle, lastCheckpointAngle);
void spinMagnI2C(const int RUN_SPEED,
                 const int TARGET_DELTA,
                 AccelStepper& stepper,
                 const int DIR,
                 const float startAngle,   // no longer needed
                 float& lastCheckpointAngle) {

  // 🔑 Initialize reference angle ONCE
  lastCheckpointAngle = getAbsAngle();

  stepper.setSpeed(DIR * RUN_SPEED);
  Serial.println("Rotating...");

  checkRotated90(DIR, startAngle, lastCheckpointAngle, TARGET_DELTA, stepper);
}
