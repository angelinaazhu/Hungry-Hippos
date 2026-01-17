#pragma once
void light_LED(Adafruit_NeoPixel& strip);
void remove_void_sample(SFE_ISL29125& RGB_sensor);
void sample_scaled_RGB(int& redVal, int& greenVal, int& blueVal,
                       SFE_ISL29125& RGB_sensor, const unsigned int redMin,
                       const unsigned int redMax, const unsigned int greenMin,
                       const unsigned int greenMax, const unsigned int blueMin,
                       const unsigned int blueMax);

// updated classify prototype for N colors
void classify(const unsigned long VOTING_WINDOW,
              const unsigned long VOTING_INTERVAL, Adafruit_NeoPixel& strip,
              SFE_ISL29125& RGB_sensor, Adafruit_7segment& hex_points,
              int& points, const double colorAvgR[], const double colorAvgG[],
              const double colorAvgB[], int NUM_COLORS,
              const char* colorNames[], const int pointsPerColor[],
              const unsigned int redMin, const unsigned int redMax,
              const unsigned int greenMin, const unsigned int greenMax,
              const unsigned int blueMin, const unsigned int blueMax);