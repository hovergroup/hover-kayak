#ifndef Roboteq_H
#define Roboteq_H

#include <Arduino.h>

class Roboteq
{
public:
  Roboteq(Stream& port);
  
  boolean doWork();
  void initialize();
  
  void setPower( int velocity );
  
  int getHeatsinkTemp() { return heatsink_temp; }
  int getInternalTemp() { return internal_temp; }
  
  float getBatteryVoltage() { return battery_voltage; }
  
  float getMotorAmps() { return motor_amps; };
  float getBatteryAmps() { return battery_amps; }
  
  boolean getStopSwitch() { return stop_switch; }
  
  int getPowerOutput() { return power_output; }
  
private:
  Stream& _port;
  static const unsigned int ROBOTEQ_READ_UPDATE = 50;
  unsigned long last_read_time;
  int read_state;
  
  int current_velocity;
  
  int internal_temp, heatsink_temp, power_output;
  float internal_voltage, battery_voltage, five_voltage, motor_amps, battery_amps;
  boolean stop_switch;
  
  char buffer[100];
  int buffer_index;
  
  void readBuffer();
  int processBuffer();
  int findLine( int index );
  void shiftBuffer( int shift );
  
  void parseVoltage( int index, int stopIndex );
  void parseTemperature( int index, int stopIndex );
  void parseMotorAmps( int index, int stopIndex );
  void parseBatteryAmps( int index, int stopIndex );
  void parseStopSwitch( int index, int stopIndex );
  void parsePowerOutput( int index, int stopIndex );
  
  void sendSpeed( int velocity );
};

#endif
