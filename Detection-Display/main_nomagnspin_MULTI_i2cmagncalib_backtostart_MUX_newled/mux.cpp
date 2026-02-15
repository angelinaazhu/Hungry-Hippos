#include "Wire.h"
#define MUX_ADDR 0x70 //TCA9548A encoder address

//helper function to help select port
// use by doing TCAsel(0) -> TCAsel(7)
void TCAsel(uint8_t i2c_bus){
  if (i2c_bus > 7){
    return;
  }

  Wire.beginTransmission (MUX_ADDR);
  Wire.write(1 << i2c_bus);
  Wire.endTransmission();
}