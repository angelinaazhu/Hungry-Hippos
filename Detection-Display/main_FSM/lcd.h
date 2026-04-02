#pragma once
#include "rgb_lcd.h"

bool startingMessage(rgb_lcd& lcd);
void gameScreen(rgb_lcd& lcd);
void coolDownMsg(rgb_lcd& lcd);
void win(rgb_lcd& lcd, int n);
void loss(rgb_lcd& lcd, int n);
void tie(rgb_lcd& lcd, int n);