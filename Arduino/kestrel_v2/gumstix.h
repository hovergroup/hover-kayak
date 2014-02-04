#include "Arduino.h"
#include "radio.h"
#include "roboteq.h"
#include "azimuth.h"
#include "tmp102.h"
#include "battery.h"

#ifndef __GUMSTIX_H
#define __GUMSTIX_H

extern RadioControl radio;
extern Roboteq roboteq;
extern Azimuth azimuth;
extern TMP102 tmp102;
extern Battery battery;

class GumstixSerial {
public:
  GumstixSerial(Stream& port);
  void doWork();
  
  int getRudderCommand() { return rudder; }
  int getThrustCommand() { return thrust; }
  boolean getCommandsAvailable() { return commands_ready; }
  
  void rolloverCounters(float elapsed_seconds);
  
  float getMotorCommandRate() { return motorCommandRate; }
  float getBadParseRate() { return badParseRate; }
  unsigned long getTotalBadParseCount() { return totalBadParseCount; }
  float getFastReportRate() { return fastReportRate; }
  float getSlowReportRate() { return slowReportRate; }
  
private:
  Stream& _port;
  static const int shortestCompletePacket = 20;
  
  // control
  int thrust, rudder;
  boolean commands_ready;
    
  unsigned long last_command_time;
  
  char buffer[100];
  int buffer_index;
  
  // general serial handling
  void readBuffer();
  int processBuffer();
  int findLine( int index );
  void shiftBuffer( int shift );
  
  // specific parses
  void parseMotorCommand(int index, int stopIndex);
  void parseRadioCommand(int index, int stopIndex);
  
  // publishing
  void publishFast();
  void publishSlow();
  
  static const int slow_publish_period = 1000;
  static const int fast_publish_period = 20;
  unsigned long last_slow_publish, last_fast_publish;
  
  float motorCommandRate, badParseRate, fastReportRate, slowReportRate;
  int motorCommandCount, badParseCount, fastReportCount, slowReportCount;
  unsigned long totalBadParseCount;
};

#endif


