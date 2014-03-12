#ifndef BATTERY_H
#define BATTERY_H

#include <Arduino.h>
#include "roboteq.h"

extern Roboteq roboteq;

enum VoltageState {
  okay,
  low_limit,
  crit_limit,
  unset
};

class Battery {
public:
  Battery() {
    m_state = okay;
    next_state = unset;
    detect_start = 0;
  }
  
  int getThrustLimit() {
    switch (m_state) {
      case okay:
        return 1000;
        break;
      case low_limit:
        return 400;
        break;
      case crit_limit:
        return 0;
        break;
    }
  }
  
  void doWork() {
    float voltage = roboteq.getBatteryVoltage();
    switch (m_state) {
      
      case okay:
        if (voltage < low_voltage) { // voltage is low
          if (next_state != low_limit) { // first time detection - set next state and record time
            next_state = low_limit;
            detect_start = millis();
          } else if (millis() - detect_start > 10000) { // voltage has been low, check if enough time has passed to advance state
            m_state = low_limit;
            next_state = unset; 
          }
        } else { // voltage not low, reset any detections
          next_state = unset;
        }
        break;
        
      case low_limit:
        if (voltage < critical_voltage) { // voltage is critical
          if (next_state != crit_limit) { // first time detection - set next state and record time
            next_state = crit_limit;
            detect_start = millis();
          } else if (millis() - detect_start > 10000) { // voltage has been critical, check if enough time has passed to advance state
            m_state = crit_limit;
            next_state = unset; 
          }
        } else if (voltage > okay_voltage) { // voltage is okay
          if (next_state != okay) { // first time detection - set next state and record time
            next_state = okay;
            detect_start = millis();
          } else if (millis() - detect_start > 10000) { // voltage has been okay, check if enough time has passed to advance state
            m_state = okay;
            next_state = unset; 
          }
        } else { // voltage is still low
          next_state = unset;
        }
        break;
        
      case crit_limit:
        if (voltage > low_voltage) { // voltage is low
          if (next_state != low_limit) { // first time detection - set next state and record time
            next_state = low_limit;
            detect_start = millis();
          } else if (millis() - detect_start > 10000) { // voltage has been low, check if enough time has passed to advance state
            m_state = low_limit;
            next_state = unset; 
          }
        } else { // voltage is still critical
          next_state = unset;
        }
        break;  
    }
  }
  
  
private:
  static const float okay_voltage = 11.7;
  static const float low_voltage = 11.4;
  static const float critical_voltage = 10.6;
  
  VoltageState m_state, next_state;
  
  unsigned long detect_start;
  
};

#endif
