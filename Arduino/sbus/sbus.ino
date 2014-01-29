#define SBUS_SIGNAL_OK          0x00
#define SBUS_SIGNAL_LOST        0x01
#define SBUS_SIGNAL_FAILSAFE    0x03

uint8_t sbus_data[25] = {
  0x0f,0x01,0x04,0x20,0x00,0xff,0x07,0x40,0x00,0x02,0x10,0x80,0x2c,0x64,0x21,0x0b,0x59,0x08,0x40,0x00,0x02,0x10,0x80,0x00,0x00};
int16_t channels[18]  = {
  1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,0,0};
int16_t servos[18]    = {
  1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,0,0};
uint8_t  failsafe_status = SBUS_SIGNAL_FAILSAFE;
int sbus_passthrough = 1;
uint8_t byte_in_sbus;
uint8_t bit_in_sbus;
uint8_t ch;
uint8_t bit_in_channel;
uint8_t bit_in_servo;
uint8_t inBuffer[25];
int bufferIndex=0;
uint8_t inData;
int toChannels = 0;
uint32_t baud = 100000;


void setup(){
  Serial.begin(115200);
  Serial3.begin(100000);
  //UCSR3C = (1<<USBS0)|(3<<UCSZ00);
  //UCSR3C |= B00110000;
  delay(500);
  Serial.println("running");
}

void loop(){
  feedLine();
  if(toChannels==1){
    update_channels();
//    update_servos();
    toChannels=0;
    
    if ( failsafe() == SBUS_SIGNAL_LOST ) {
      Serial.println("signal lost");
    } else if ( failsafe() == SBUS_SIGNAL_FAILSAFE ) {
      Serial.println("failsafe");
    } else {
      Serial.print(channels[0]);
      Serial.print(" ");
      Serial.print(channels[1]);
      Serial.print(" ");
      Serial.print(channels[2]);
      Serial.print(" ");
      Serial.print(channels[3]);
      Serial.print(" ");
      Serial.print(channels[4]);
      Serial.print(" ");
      Serial.print(channels[5]);
      Serial.print(" ");
      Serial.println(channels[6]);
    }
  }  
  
  //update_servos();
}





int16_t channel(uint8_t ch) {
  // Read channel data
  if ((ch>0)&&(ch<=16)){
    return channels[ch-1];
  }
  else{
    return 1023;
  }
}
uint8_t digichannel(uint8_t ch) {
  // Read digital channel data
  if ((ch>0) && (ch<=2)) {
    return channels[15+ch];
  }
  else{
    return 0;
  }
}
//void servo(uint8_t ch, int16_t position) {
//  // Set servo position
//  if ((ch>0)&&(ch<=16)) {
//    if (position>2048) {
//      position=2048;
//    }
//    servos[ch-1] = position;
//  }
//}
//void digiservo(uint8_t ch, uint8_t position) {
//  // Set digital servo position
//  if ((ch>0) && (ch<=2)) {
//    if (position>1) {
//      position=1;
//    }
//    servos[15+ch] = position;
//  }
//}
uint8_t failsafe(void) {
  return failsafe_status;
}
//
//void passthroughSet(int mode) {
//  // Set passtrough mode, if true, received channel data is send to servos
//  sbus_passthrough = mode;
//}

//int passthroughRet(void) {
//  // Return current passthrough mode
//  return sbus_passthrough;
//}
//void update_servos(void) {
//  // Send data to servos
//  // Passtrough mode = false >> send own servo data
//  // Passtrough mode = true >> send received channel data
//  uint8_t i;
//  if (sbus_passthrough==0) {
//    // clear received channel data
//    for (i=1; i<24; i++) {
//      sbus_data[i] = 0;
//    }
//
//    // reset counters
//    ch = 0;
//    bit_in_servo = 0;
//    byte_in_sbus = 1;
//    bit_in_sbus = 0;
//
//    // store servo data
//    for (i=0; i<176; i++) {
//      if (servos[ch] & (1<<bit_in_servo)) {
//        sbus_data[byte_in_sbus] |= (1<<bit_in_sbus);
//      }
//      bit_in_sbus++;
//      bit_in_servo++;
//
//      if (bit_in_sbus == 8) {
//        bit_in_sbus =0;
//        byte_in_sbus++;
//      }
//      if (bit_in_servo == 11) {
//        bit_in_servo =0;
//        ch++;
//      }
//    }
//
//    // DigiChannel 1
//    if (channels[16] == 1) {
//      sbus_data[23] |= (1<<0);
//    }
//    // DigiChannel 2
//    if (channels[17] == 1) {
//      sbus_data[23] |= (1<<1);
//    }
//
//    // Failsafe
//    if (failsafe_status == SBUS_SIGNAL_LOST) {
//      sbus_data[23] |= (1<<2);
//    }
//
//    if (failsafe_status == SBUS_SIGNAL_FAILSAFE) {
//      sbus_data[23] |= (1<<2);
//      sbus_data[23] |= (1<<3);
//    }
//  }
//  // send data out
//  //serialPort.write(sbus_data,25);
// for (i=0;i<25;i++) {
//    port0.write(sbus_data[i]);
//  }
//}
void update_channels(void) {
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
void feedLine(){
  while(Serial3.available()){
    inData = Serial3.read();
    //Serial.print(inData);
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
