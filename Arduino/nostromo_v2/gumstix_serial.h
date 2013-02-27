#include "Arduino.h"

#ifndef __GUMSTIX_SERIAL_H
#define __GUMSTIX_SERIAL_H

class GUMSTIX_SERIAL {
public:
  GUMSTIX_SERIAL(Stream& port);
  void doWork();
  void initialize();
  
  int getRudderCommand() { return rudder; }
  int getThrustCommand() { return thrust; }
  unsigned long getLastCommandTime() { return last_command_time; }
  
private:
  Stream& _port;
  static const int shortestCompletePacket = 20;
  
  int thrust, rudder;
  unsigned long last_command_time;
  
  char buffer[100];
  int buffer_index;
  
  void readBuffer();
  int processBuffer();
  int findLine( int index );
  void shiftBuffer( int shift );
  
  void parseMotorCommand( int index, int stopIndex );
};

#endif


