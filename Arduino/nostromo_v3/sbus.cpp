#include "sbus.h"

SBUS::SBUS(Stream& port) : _port( port ) {
  uint8_t temp_sbus_data[25] = { 0x0f,0x01,0x04,0x20,0x00,0xff,0x07,0x40,0x00,0x02,0x10,
    0x80,0x2c,0x64,0x21,0x0b,0x59,0x08,0x40,0x00,0x02,0x10,0x80,0x00,0x00};
  memcpy(&temp_sbus_data[0], &sbus_data[0], 25);
  uint16_t temp_channels[18]  = { 1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,0,0};
  memcpy(&temp_channels[0], &channels[0], 36);
failsafe_status = SBUS_SIGNAL_FAILSAFE;
sbus_passthrough = 1;
bufferIndex=0;
toChannels = 0;
}

int SBUS::getThrust() {
  int thrust_channel = constrain( channels[1], 368, 1650);
  return map(thrust_channel, 1630, 368, -1000, 1000);
}

int SBUS::getRudder() {
  int azimuth_channel = constrain( channels[0], 364, 1686);
  return map( azimuth_channel , 364, 1686, -60, 60);
}

void SBUS::doWork() {
  if (iterate()) {
    if ( failsafe_status == SBUS_SIGNAL_OK ) {
      use_rc = true;
    } else {
      use_rc = false;
    }
    
    if (channels[6] < 1000) thrust_enable = true;
    else thrust_enable = false; 
    
    if (channels[4] < 1000) serial_enable = true;
    else serial_enable = false;
  } 
}

boolean SBUS::iterate() {
  feedLine(); 
  if(toChannels==1){
    update_channels();
    toChannels=0;
    
    return true;
  }
  return false;
}

void SBUS::feedLine(){
  while(_port.available()){
    inData = _port.read();
    if (inData == 0x0f){
      bufferIndex = 0;
      inBuffer[bufferIndex] = inData;
      inBuffer[24] = 0xff;
    }
    else{
      bufferIndex ++;
      inBuffer[bufferIndex] = inData;
    }
    if(inBuffer[0]==0x0f & inBuffer[24] == 0x00){

      memcpy(sbus_data,inBuffer,25);
      toChannels = 1;
      return;
    }
  }
}

void SBUS::update_channels() {
  uint8_t i;
  uint8_t sbus_pointer = 0;
  // clear channels[]
  for (i=0; i<16; i++) {
    channels[i] = 0;
  }

  // reset counters
  byte_in_sbus = 1;
  bit_in_sbus = 0;
  ch = 0;
  bit_in_channel = 0;

  // process actual sbus data
  for (i=0; i<176; i++) {
    if (sbus_data[byte_in_sbus] & (1<<bit_in_sbus)) {
      channels[ch] |= (1<<bit_in_channel);
    }
    bit_in_sbus++;
    bit_in_channel++;

    if (bit_in_sbus == 8) {
      bit_in_sbus =0;
      byte_in_sbus++;
    }
    if (bit_in_channel == 11) {
      bit_in_channel =0;
      ch++;
    }
  }
  // DigiChannel 1
  if (sbus_data[23] & (1<<0)) {
    channels[16] = 1;
  }
  else{
    channels[16] = 0;
  }
  // DigiChannel 2
  if (sbus_data[23] & (1<<1)) {
    channels[17] = 1;
  }
  else{
    channels[17] = 0;
  }
  // Failsafe
  failsafe_status = SBUS_SIGNAL_OK;
  if (sbus_data[23] & (1<<2)) {
    failsafe_status = SBUS_SIGNAL_LOST;
  }
  if (sbus_data[23] & (1<<3)) {
    failsafe_status = SBUS_SIGNAL_FAILSAFE;
  }

}
