// from youtube: https://www.youtube.com/watch?v=6wuInF9Yw08
// from chatgpt: https://chatgpt.com/share/685f2bb7-00c0-8009-8b3c-65759d2ae005

// tells you when to count the score up by 1

#define LIMIT_SW 50
//#define LED 13

int score = 0;
bool curr = false; // curr = false means sw not pressed
bool prev = false; // prev = false means sw not pressed in previous state

void setup(){
	pinMode(LIMIT_SW, INPUT_PULLUP);
	//pinMode(LED, OUTPUT); // LED for debug
	
	//digitalWrite(LED, LOW); //LED init off

  Serial.begin(9600);
}

void loop(){
	
	//check state of limit sw
	if(digitalRead(LIMIT_SW) == LOW){ // NO connected to COM -> GND -> pressed
		//digitalWrite(LED, HIGH); // LED on
		//Serial.println("PRESSED"); // ball is there
		
		curr = true;
		
	} else { // NO connected to 5V -> unpressed (pulled up to ~5V)
		//digitalWrite(LED, LOW); // LED off
		//Serial.println("unpressed"); // ball is not there
		
		curr = false;
	}
	
	//check if should count score
	if(curr == true && prev == false){ // ball JUST pressed limit switch -> count up
		score++;
    Serial.print("score: ");
    Serial.println(score);
	} // else never count score up
	
	prev = curr; //set previous state to current state -> time passing
		
	delay(100);
}