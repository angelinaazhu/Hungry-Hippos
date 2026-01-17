#include <Wire.h> 

//Magnetic sensor things
// int magnetStatus = 0; //value of the status register (MD, ML, MH)
//                       //tells if magnet is detected & if strength OK

// int lowbyte; //raw angle 7:0
// word highbyte; //raw angle 7:0 and 11:8
// int rawAngle; //final raw angle number (0-4095)
float ABSOLUTEANGLE; //raw angle in degrees (360/4096 * [value between 0-4095])
float STARTANGLE = 0.0; //starting angle
float RELATIVEANGLE = 0.0; //relative to startAngle: current angle -  startAngle 
float lastCheckpointAngle = 0.0; //to keep track of when last the motor stopped spinning (completed 90)
const float CHECKPOINT = 90.0; //checkpoint every 90 degrees

void setup(){
  Serial.begin(9600);
  Wire.begin(); //start i2C  
  Wire.setClock(800000L); //fast clock 800kHz freq

  checkMagnetPresence(); //check the magnet (blocks until magnet is found)

  // getAbsAngle(); //make a reading -> update absAngle
  //startAngle = getAbsAngle(); //make a reading -> update absAngle

  Serial.println("Welcome!"); //print a welcome message  
  Serial.println("AS5600"); //print a welcome message
  delay(3000);

  // Serial.print("start angle: ");
  // Serial.println(startAngle);
  // delay(100);

  //reads current to set checkpoint for 90 degree rotation detection
  ABSOLUTEANGLE = getAbsAngle();
  STARTANGLE = ABSOLUTEANGLE;
  RELATIVEANGLE = getRelativeAngle(ABSOLUTEANGLE, STARTANGLE);
  lastCheckpointAngle = RELATIVEANGLE; // initialize last checkpoint angle to be relative starting angle
  // Serial.print("lastcheckpointangle"); Serial.println(lastCheckpointAngle);
  // Serial.print("absoluteangle"); Serial.println(ABSOLUTEANGLE);
  // Serial.print("startangle"); Serial.println(STARTANGLE);
  // Serial.print("relativeangle"); Serial.println(RELATIVEANGLE);
  // delay(3000);
}

void loop(){    
  if(checkRotated90(-1, STARTANGLE, lastCheckpointAngle, CHECKPOINT)){
    Serial.println("stop spinning");
  }
  delay(100); //wait a little - adjust it for "better resolution"
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
  
  Serial.print("Relative angle: ");
  Serial.println(relAngle, 2); //print the corrected/tared angle  
  return relAngle;
}

bool checkRotated90(int direction, float startAngle, float &lastCheckpointAngle1, float CHECKPOINT1) {
  float absAngle, relAngle;
  // direction: 1 for CW (increasing angle), -1 for CCW (decreasing angle)
  while (true) {
    absAngle = getAbsAngle();
    relAngle = getRelativeAngle(absAngle, startAngle);
    float delta = relAngle - lastCheckpointAngle1;
    // handle wrap-around (shortest signed difference)
    if (delta > 180.0) delta -= 360.0;
    if (delta < -180.0) delta += 360.0;
    // Only trigger if rotation is at least 90° in the desired direction
    if (direction * delta >= CHECKPOINT1) { //-1 cancels out the negative delta if CCW
      Serial.print("Rotated 90 degrees! Delta: ");
      Serial.println(delta, 2);
      lastCheckpointAngle1 = relAngle;
      // break;
      return true;
    }
    delay(10); // small delay to avoid busy-waiting
  }
}


void checkMagnetPresence(){  
    int magnetStatus = 0; //value of the status register (MD, ML, MH)
                        //tells if magnet is detected & if strength OK

  //This function runs in the setup() and it locks the MCU until the magnet is not positioned properly

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