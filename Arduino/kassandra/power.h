#include "Arduino.h"
#include "configuration.h"

#ifndef __POWER_H
#define __POWER_H

class Power{
public:
  Power();
  void doWork();
  static float getVoltage();
  static void set12V( boolean onoff );
  static void set5V( boolean onoff );
  static void setAux( boolean onoff );

private:
  static const float gain = voltageDividerGain;
};

#endif

