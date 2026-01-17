#pragma once
void mini_countdown(Adafruit_7segment& hex_timer, unsigned long& currentMillis, unsigned long& previousMillis, unsigned int& secondsUntilGo, bool& start);
void countdown(Adafruit_7segment& hex_timer, unsigned long& currentMillis, unsigned long& previousMillis, unsigned int& remainingSeconds);