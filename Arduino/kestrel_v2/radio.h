#ifndef RADIO_H
#define RADIO_H

#include <Arduino.h>

#define radioSwitchPin 58

enum RadioPower {
  bullet,
  freewave
};

class RadioControl
{
public:
  RadioControl() {}
  
  void powerBullet() {
    digitalWrite(radioSwitchPin, LOW);
    m_power = bullet;
  }
  
  void powerFreewave() {
    digitalWrite(radioSwitchPin, HIGH);
    m_power = freewave;
  }
    
  RadioPower getRadioPower() { return m_power; }
  
  boolean getPowerIsBullet() {
    if (m_power == bullet) return true;
    else return false;
  }
  
private:
  RadioPower m_power;
  
};

#endif
