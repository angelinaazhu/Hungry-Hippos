#define SENSORA 4
#define SENSORB 5

int sensorStateA = 0;
int sensorStateB = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(SENSORA, INPUT);     
  digitalWrite(SENSORA, HIGH); // turn on the pullup

  pinMode(SENSORB, INPUT);     
  digitalWrite(SENSORB, HIGH); // turn on the pullup
  
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  sensorStateA = digitalRead(SENSORA); // 1 = unbroken, 0 = broken

  if(sensorStateA == 0) {
    Serial.println("A: BROKEN");
  } else if (sensorStateA == 1){
    Serial.println("A: UNBROKEN");
  }

  sensorStateB = digitalRead(SENSORB); // 1 = unbroken, 0 = broken

  if(sensorStateB == 0) {
    Serial.println("B: BROKEN");
  } else if (sensorStateB == 1){
    Serial.println("B: UNBROKEN");
  }

  delay(1000);
}

