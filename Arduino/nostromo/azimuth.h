#include "Arduino.h"
#include "configuration.h"

#include <Servo.h>

#ifndef __AZIMUTH_H
#define __AZIMUTH_H

class Azimuth{
public:
  Azimuth();
  
  void initialize();
  void setAngle( int angle );
  
  int getCurrentAngle() { return current_angle; }

private:
  Servo azimuth_servo;
  
  int current_angle;
};

#endif
