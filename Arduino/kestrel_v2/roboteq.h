#ifndef Roboteq_H
#define Roboteq_H

#include <Arduino.h>

class Roboteq
{
public:
  Roboteq(Stream& port);
  
  boolean doWork();
  
  void setPower( int velocity );
  
  int getHeatsinkTemp() { return heatsink_temp; }
  int getInternalTemp() { return internal_temp; }
  
  float getBatteryVoltage() { return battery_voltage; }
  
  float getMotorAmps() { return motor_amps; };
  float getBatteryAmps() { return battery_amps; }
  
  boolean getStopSwitch() { return stop_switch; }
  
  int getPowerOutput() { return power_output; }
  
  void rolloverCounters(float elapsed_seconds);
  unsigned long getTotalBadParses() { return total_bad_parse_count; }
  
  float getSpeedRate() { return speedRate; }
  float getVoltageRate() { return voltageRate; }
  float getTmpRate() { return tempRate; }
  float getMotorAmpRate() { return motorAmpRate; }
  float getBatteryAmpRate() { return batteryAmpRate; }
  float getStopSwitchRate() { return stopSwitchRate; }
  float getPowerOutRate() { return powerOutRate; }
  float getBadParseRate() { return badParseRate; }
  
  void setLights(boolean onoff);
  void toggleLights();
  boolean getLightsState() { return lights_state; }
  
private:
  Stream& _port;
  static const unsigned int ROBOTEQ_READ_UPDATE = 20;
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
  
  unsigned long total_bad_parse_count;
  
  float speedRate, voltageRate, tempRate, motorAmpRate, batteryAmpRate, stopSwitchRate, powerOutRate, badParseRate;
  
  int speedCount, voltageCount, tempCount, motorAmpCount, batteryAmpCount, stopSwitchCount, powerOutCount, badParseCount;
  
  boolean lights_state;
};

#endif

