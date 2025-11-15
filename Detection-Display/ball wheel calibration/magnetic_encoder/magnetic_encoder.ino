void setup() {
  Serial.begin(9600);
}

void loop() {
  int rawValue = analogRead(A0);  // 0–1023 on Arduino Mega

  Serial.print("Raw analog value: ");
  Serial.println(rawValue);

  delay(20);
}
