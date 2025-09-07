#define SENSORPIN 4

int sensorState = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(SENSORPIN, INPUT);     
  digitalWrite(SENSORPIN, HIGH); // turn on the pullup
  
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  sensorState = digitalRead(SENSORPIN); // 1 = unbroken, 0 = broken

  if(sensorState == 0) {
    Serial.println("BROKEN");
  } else if (sensorState == 1){
    Serial.println("UNBROKEN");
  }
  delay(100);
}
