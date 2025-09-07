#include <Adafruit_NeoPixel.h>

// Pin where DIN (pin 4 of WS2812D-265) is connected
#define LED_PIN 6    

// Number of LEDs in your strip (or single LED)
#define NUM_LEDS 2  

// Create NeoPixel object
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  strip.begin();        // Initialize LED
  strip.show();         // Turn off all LEDs initially
  strip.setBrightness(255); // Max brightness (0-255)
  
  // Set LED #0 to white (R=255, G=255, B=255)
  strip.setPixelColor(0, strip.Color(255, 255, 255));
  strip.setPixelColor(1, strip.Color(255, 255, 255));
  //strip.setPixelColor(2, strip.Color(255, 255, 255));
  //strip.setPixelColor(3, strip.Color(255, 255, 255));
  strip.show();         // Update LED
}

void loop() {
  // Nothing here – LED stays white
}
