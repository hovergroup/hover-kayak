#include <Wire.h>
#include "Arduino.h"
#include "configuration.h"

#ifndef __SENSOR_BOARD_H
#define __SENSOR_BOARD_H

class Temp{
public:
  Temp() {}
  static float getTemp();

private:
  static const float gain = 100; // degrees celcius per volt
  static const float offset = temperatureCalibrationOffset;
};

class Humidity{
public:
  Humidity() {}
  static float getHumidity(); // return % relative humidity

private:
};

class Gyro {
public:
  Gyro(int gyro_pin);
  void initialize();
  float getOmega();
  
  float heading, pitch, roll, acc_x, acc_y, acc_z;
private:
  int address, pin, offset;
};

class Compass {
public:
  Compass();
  void updateHPR();
  void updateAcc();
  
  void startCalibration();
  void stopCalibration();
  
  float heading, pitch, roll, acc_x, acc_y, acc_z;
  boolean inCalibrationMode;
private:
  int compassAddress;
  unsigned char responseBytes[6];
};

#endif

