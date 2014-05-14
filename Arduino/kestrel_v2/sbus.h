#ifndef SBUS_H
#define SBUS_H

#include "Arduino.h"

#define SBUS_SIGNAL_OK          0x00
#define SBUS_SIGNAL_LOST        0x01
#define SBUS_SIGNAL_FAILSAFE    0x03

class SBUS {
public:
  SBUS(Stream& port);
int16_t channels[18];
uint8_t  failsafe_status;
   
  void doWork();
  
  int getThrust();
  int getRudder();
  boolean getRcAvailable() { return use_rc; }
  boolean getThrustEnable() { return thrust_enable; }
  boolean getSerialEnable() { return serial_enable; }
  
private:
  boolean use_rc, thrust_enable, serial_enable, rc_lost;
  unsigned long rc_active_time;
  boolean iterate();

Stream& _port;

uint8_t sbus_data[25];
int sbus_passthrough;
uint8_t byte_in_sbus;
uint8_t bit_in_sbus;
uint8_t ch;
uint8_t bit_in_channel;
uint8_t bit_in_servo;
uint8_t inBuffer[25];
int bufferIndex;
uint8_t inData;
int toChannels;

  void update_channels();
  void feedLine();
};

#endif
