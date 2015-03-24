#include "Arduino.h"

#ifndef __GUMSTIX_SERIAL_H
#define __GUMSTIX_SERIAL_H

class GUMSTIX_SERIAL {
public:
  GUMSTIX_SERIAL();
  void doWork();
  void initialize();
  
  int motor_thrust, motor_rudder;
  boolean new_motor_command, compass_calibration_signal;
  unsigned long last_motor_command_time;
  
private:
  static const int shortestCompletePacket = 20;
  
  char buffer[256];
  int index, last_integer_end;
  
  void readBuffer();
  void parseBuffer();
  void parseMotorCommand(int startIndex);
  int parseNext();
};

#endif

