const int buttonpin = 6;

void setup(){
  Serial.begin(9600);
  pinMode(buttonpin, INPUT_PULLUP);
}

void loop (){
  int buttonstate = digitalRead(buttonpin);
  if (buttonstate == LOW){
    Serial.println("ON");
  } else {
    Serial.println("OFF");
  }
  delay(100);
}