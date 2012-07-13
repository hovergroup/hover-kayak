#include "Arduino.h"
#include "configuration.h"

#include "Sabertooth.h"
#include <Servo.h>

#ifndef __MOTOR_H
#define __MOTOR_H

class Motor{
public:
  Motor();
  
  boolean doWork();
  
  void initialize();
  void setPWM( int percentage );
  void setAngle( int angle );

private:
  Sabertooth ST;

  Servo azimuth_servo;
  unsigned long last_time;
  double current_percent;
  int target_percent;
  boolean enabled;
  
  void outputPWM( int percent );
};

#endif
