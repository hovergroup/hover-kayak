#include "Arduino.h"
#include "configuration.h"

#include <Wire.h>

#ifndef __TMP102_H
#define __TMP102_H

class TMP102 {
public:
  TMP102() {}
  
  float getTemp() { return temp; }
  void updateReading();
  
private:
  float temp;
  
  const static int sensorAddress = (0x91>>1);
};

#endif
