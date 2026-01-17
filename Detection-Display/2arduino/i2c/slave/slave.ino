// Chapter 7 - Communications
// I2C Slave
// By Cornel Amariei for Packt Publishing

// Include the required Wire library for I2C
#include <Adafruit_NeoPixel.h>
#include <Wire.h>

const uint8_t I2C_ADDR = 9;
const uint8_t PIXEL_PIN = 6;  // move off pin 13
Adafruit_NeoPixel strip(1, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

void light_LED(Adafruit_NeoPixel& strip){
  strip.setBrightness(255); // brightness [0-255]
  strip.setPixelColor(0, strip.Color(255, 255, 255)); //LED0: R,G,B=255,255,255
  //strip.setPixelColor(1, strip.Color(255, 255, 255)); //LED1: R,G,B=255,255,255
  strip.show(); // update LED with set brightness & colour
}

void no_light_LED(Adafruit_NeoPixel& strip){
  strip.setBrightness(0); // brightness [0-255]
  strip.setPixelColor(0, strip.Color(255, 255, 255)); //LED0: R,G,B=255,255,255
  //strip.setPixelColor(1, strip.Color(255, 255, 255)); //LED1: R,G,B=255,255,255
  strip.show(); // update LED with set brightness & colour
}

volatile uint8_t x = 0;

void receiveEvent(int count) {
  while (Wire.available()) {
    x = Wire.read();  // keep latest byte
  }
}

void setup() {
  strip.begin();
  Wire.begin(I2C_ADDR);
  Wire.onReceive(receiveEvent);
}

void loop() {
  uint8_t val = x;  // copy volatile once

  if (val == 0) {
    no_light_LED(strip);
    strip.show();
  } else if (val == 1) {
    light_LED(strip);
  }
}
