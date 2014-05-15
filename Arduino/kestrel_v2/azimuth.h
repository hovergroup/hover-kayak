#ifndef __AZIMUTH_H
#define __AZIMUTH_H

#include <Servo.h>
#include <Arduino.h>

class Azimuth{
public:
  Azimuth();
  
  void doWork();
  
  void initialize();
  void setAngle( int angle );
  
  int getCurrentAngle() { return (int) angle_estimate; }
  
  void setEStop(boolean stopped);
  void setThrustLimit(int limit);

private:
  Servo azimuth_servo;
  
  void updatePower();
  
  int current_angle;
  double angle_estimate;
  unsigned long last_work_time;
  boolean e_stop_on, limit_at_zero, power_off;
};

#endif

