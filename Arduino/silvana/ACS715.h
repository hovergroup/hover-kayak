#include "Arduino.h"
#include "configuration.h"

class ACS715 {
public:
  int getMilliAmps();
private:
  static const int pin = 14;

};
