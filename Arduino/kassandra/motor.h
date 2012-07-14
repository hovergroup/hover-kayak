#include "Arduino.h"
#include "configuration.h"
#include <Servo.h>

#ifndef __MOTOR_H
#define __MOTOR_H

class Motor{
public:
  Motor();
  
  void doWork();
  
  void initialize();
  void setDirection( boolean forward );
  void setPWM( int percentage );
  void setAngle( int angle );
  void setVoltage( double voltage );
  void setEnable( boolean onoff );
  double readCurrent();

private:
  Servo azimuth_servo;
  unsigned long last_time;
  double current_percent, batt_voltage;
  int target_percent, base_current;
  boolean enabled;
  
  void outputPWM( int percent );
};

#endif
