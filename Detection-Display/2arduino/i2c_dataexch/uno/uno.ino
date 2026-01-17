//wire lib
#include <Wire.h>
#define UNO_ADDR 9
#define ANSWERSIZE 5

String answer = "Hello"; //answer

void setup(){
    //give it the slave address, so the wire lib knows to run in slave mode
  Wire.begin(UNO_ADDR); 

  //define func for when data is requested from master 
  //uno -> master
  Wire.onRequest(requestEvent);

  //define func for when data is received from master
  //master -> uno
  Wire.onReceive(receiveEvent);

  Serial.begin(9600);
  Serial.println("I2C uno demo");
}

void receiveEvent(){ //when we receive data from master
  while (0 < Wire.available()){ //while 
    byte x = Wire.read(); //read data and assign it to byte x
  }

  //print to serial monitor
  Serial.println("[U] Uno got data from Mega (M->U)");
}

void requestEvent(){
  byte response[ANSWERSIZE]; //byte array

  for (byte i=0; i<ANSWERSIZE; i++){ //format "Hello" into an array)
    response[i] = (byte) answer.charAt(i);
  }

  //send response back to master
  Wire.write(response, sizeof(response));

  //print to serial monitor
  Serial.println("[U] Mega requested data from Uno (U->M)");

}

void loop(){
  delay(50);
}