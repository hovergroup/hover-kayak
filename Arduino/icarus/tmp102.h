#include "Arduino.h"

#include <Wire.h>

#ifndef __TMP102_H
#define __TMP102_H

class TMP102 {
public:
  TMP102() {}
  
  float getTemp() { return temp; }
  void updateReading() {
    // step 1: request reading from sensor 
    Wire.requestFrom(sensorAddress,2); 
    if (2 <= Wire.available())  // if two bytes were received 
    {
      int temperature;
      byte msb, lsb;
      
      msb = Wire.read();  // receive high byte (full degrees)
      lsb = Wire.read();  // receive low byte (fraction degrees) 
      temperature = ((msb) << 4);  // MSB
      temperature |= (lsb >> 4);   // LSB
      
      temp = temperature*0.0625;
    }
  }
  
private:
  float temp;
  
  const static int sensorAddress = (0x91>>1);
};

#endif

