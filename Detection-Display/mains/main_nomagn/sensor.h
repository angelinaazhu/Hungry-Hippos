#pragma once
void light_LED(Adafruit_NeoPixel &strip);
void remove_void_sample(SFE_ISL29125& RGB_sensor);
void sample_scaled_RGB(int& redVal, int& greenVal, int& blueVal, SFE_ISL29125& RGB_sensor,
  const unsigned int redMin, const unsigned int redMax,
  const unsigned int greenMin, const unsigned int greenMax,
  const unsigned int blueMin, const unsigned int blueMax);
void classify(const unsigned long VOTING_WINDOW, const unsigned long VOTING_INTERVAL, Adafruit_NeoPixel& strip, SFE_ISL29125& RGB_sensor, Adafruit_7segment& hex_points, int& points,
  const double envAvgR, const double envAvgG, const double envAvgB,
  const double blueBallAvgR, const double blueBallAvgG, const double blueBallAvgB,
  const double yellowBallAvgR, const double yellowBallAvgG, const double yellowBallAvgB,
  const unsigned int redMin, const unsigned int redMax,
  const unsigned int greenMin, const unsigned int greenMax,
  const unsigned int blueMin, const unsigned int blueMax);