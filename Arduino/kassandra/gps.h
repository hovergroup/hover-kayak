#include "Arduino.h"


#ifndef __GPS_H
#define __GPS_H

#define GPS_SERIAL Serial2

class GPS {
public:
  GPS();
  
  void initialize();
  void doWork();
private:
  int buffer_index, startIndex;
  char buffer [150];
  
  void forwardGPS( int stopIndex );
};

#endif


