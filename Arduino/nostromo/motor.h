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
  
  void limitPWM( int percentage );
  
  int getCurrentPWM() { return current_output; }
  int getCurrentAngle() { return current_angle; }

private:
  // our actuators
  Sabertooth ST;
  Servo azimuth_servo;
   
  // used for thruster slew rate
  unsigned long last_time;
  double current_percent;
  
  int target_percent, limit_percent, current_angle, current_output;
  
  // standard command for sending updating pwm
  void outputPWM( int percent );
  
  // for snapping straight to target, such as with e-stop
  void snapToZero();
  
  // enforce turn thrust limiting
  int turnLimit( int percent );
};

#endif
