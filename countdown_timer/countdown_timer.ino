unsigned long previousMillis = 0;
const unsigned int startSeconds = 5;  // 2 minutes
unsigned int remainingSeconds = startSeconds;

// compute minutes and seconds
unsigned int minutes = remainingSeconds / 60;
unsigned int seconds = remainingSeconds % 60;


int min10 = minutes / 10;
int min1  = minutes % 10;
int sec10 = seconds / 10;
int sec1  = seconds % 10;


void setup() {
  Serial.begin(9600);
}

void loop() {
  unsigned long currentMillis = millis();

  // every 1000 ms, decrement remainingSeconds (if > 0) and print
  if (currentMillis - previousMillis >= 1000) {
    previousMillis = currentMillis;
    
    // compute minutes and seconds
    minutes = remainingSeconds / 60;
    seconds = remainingSeconds % 60;

    // split into tens/ones if you really want digits
    min10 = minutes / 10;
    min1  = minutes % 10;
    sec10 = seconds / 10;
    sec1  = seconds % 10;

    if (remainingSeconds == startSeconds) {
      Serial.println("GAME STARTS!");
    } 

    // print as mm:ss
    Serial.print(min10);
    Serial.print(min1);
    Serial.print(':');
    Serial.print(sec10);
    Serial.println(sec1);

    if (remainingSeconds > 0) {
      --remainingSeconds;
    } else {
      Serial.println("TIME'S UP!");
      while (1) {}
    }
  
  }

  
}