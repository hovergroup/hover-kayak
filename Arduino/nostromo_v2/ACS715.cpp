#include "ACS715.h"

int ACS715::getMilliAmps() {
  int val = analogRead(pin);
  int millivolts = map(val, 0, 1023, 0, 5000);
  return (millivolts-500)*7.52;
}
