#pragma once
bool mini_countdown(Adafruit_7segment& hex_timer, unsigned long& currentMillis, unsigned long& previousMillis, unsigned int& secondsUntilGo, bool& start);
bool countdown(Adafruit_7segment& hex_timer1, Adafruit_7segment& hex_timer2, unsigned long& currentMillis, unsigned long& previousMillis, unsigned int& remainingSeconds);