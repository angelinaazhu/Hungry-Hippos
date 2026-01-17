//wire for i2c
#include <Wire.h>
#define UNO_ADDR 9 //uno i2c address
#define ANSWERSIZE 5 //answer size

void setup(){
  Wire.begin(); //no parameter, default runs as master
  Serial.begin(9600);
  Serial.println("I2C mega demo");
}

void loop(){
  delay(50);
  Serial.println("[M] Writing data to uno (M->U)");
  Wire.beginTransmission(UNO_ADDR); //begin transmitting to uno, w its addr
  Wire.write(0); //write a 0 to uno
  Wire.endTransmission(); //end trans

  Serial.println("[M] Looking for data from uno"); //looking for data back from uno
  Wire.requestFrom(UNO_ADDR, ANSWERSIZE); //read back 5 characters (5 bytes)

  //add characters to string
  String response = ""; //empty string to put uno response
  while (Wire.available()){ //while wire is available (not end)
    char b = Wire.read(); //read a character/byte from the wire
    response += b; //append it to the string
  }

  Serial.println(response); //print the response
}