#include "Arduino.h"
#include "configuration.h"

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
  boolean getUseRC() { return use_rc; }
  boolean getSerialEnable();
  
private:
  boolean use_rc;
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

