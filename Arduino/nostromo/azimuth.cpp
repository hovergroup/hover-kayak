#include "azimuth.h"

Azimuth::Azimuth() { 
  current_angle = 0;
}

void Azimuth::initialize() {
  azimuth_servo.attach(azimuthServoPinNumber);
  setAngle( 0 );
}

void Azimuth::setAngle( int angle ) {
  // constrain agnel
  int set_angle = constrain( angle, -60, 60);
  current_angle = set_angle;
  // map angle to microseconds
  int microseconds = map(set_angle, -60, 60, azimuthCenter-azimuth60Range, azimuthCenter+azimuth60Range);
  // send to servo
  azimuth_servo.writeMicroseconds( microseconds ); 
}
