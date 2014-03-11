#include "azimuth.h"

#define azimuthServoPinNumber A0
#define servoPowerRelayPin 57

#define azimuthCenter 1620
#define azimuth60Range 587

Azimuth::Azimuth() { 
  current_angle = 0;
  e_stop_on = true;
  limit_at_zero = false;
  power_off = true;
}

void Azimuth::initialize() {
  pinMode(servoPowerRelayPin, OUTPUT);
  digitalWrite(servoPowerRelayPin, HIGH);
  azimuth_servo.attach(azimuthServoPinNumber);
  setAngle( 0 );
}

void Azimuth::setAngle( int angle ) {
  // constrain angle
  int set_angle = constrain( angle, -60, 60);
  current_angle = set_angle;
  // map angle to microseconds
  int microseconds = map(set_angle, -60, 60, azimuthCenter-azimuth60Range, azimuthCenter+azimuth60Range);
  // send to servo
  azimuth_servo.writeMicroseconds( microseconds ); 
}

void Azimuth::setEStop(boolean stopped) {
  if (stopped != e_stop_on) {
    e_stop_on = stopped;
  }
  updatePower();
}

void Azimuth::setThrustLimit(int limit) {
  if (limit == 0)
    limit_at_zero = true;
  else
    limit_at_zero = false;
  updatePower();
}

void Azimuth::updatePower() {
  if (e_stop_on || limit_at_zero) { // turn off servo if either e-stop is on or thrust limit is 0
    if (!power_off) { // only write to pin if toggling power
      digitalWrite(servoPowerRelayPin, HIGH);
      power_off = true;
    }
  } else {
    if (power_off) {
      digitalWrite(servoPowerRelayPin, LOW);
      power_off = false;
    }
  }
}
