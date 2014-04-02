#ifndef LIGHTS_H
#define LIGHTS_H

#include <Arduino.h>

extern Roboteq roboteq;



class Lights
{
public:
  Lights() {
    last_event = 0;
    state =0;
    enabled = false;
    estop = true;
    timings[0] = 2000;
    timings[1] = 100;
    timings[2] = 75;
    timings[3] = 100;
    timings[4] = 350;
    timings[5] = 100;
  }
  
  void setEStop(boolean b) {
    if (b == estop) {
      return;
    } else {
      estop = b;
      setInterfaceEnable(!b);
    }
  }
  
  void setInterfaceEnable(boolean b) {
    if (b != enabled) {
      enabled = b;
      if (!enabled) {
        roboteq.setLights(false);
      } else {
        roboteq.setLights(false);
        state = 0;
        last_event = millis();
      }
    }
  }
  
  boolean getEnabled() { return enabled; }

  void doWork() {
    if (!enabled) return;
    if (millis()-last_event > timings[state]) {
      roboteq.toggleLights();
      last_event = millis();
      state++;
    }
    if (state == num_timings) {
      state = 0;
      roboteq.setLights(false);
    }
  }
  
private:
  static const int num_timings = 6;
  int timings[num_timings];

  int state;
  unsigned long last_event;
  boolean estop, enabled;
};

#endif
