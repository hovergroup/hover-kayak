#include "tmp102.h"

void TMP102::updateReading() {
  // step 1: request reading from sensor 
  Wire.requestFrom(sensorAddress,2); 
  if (2 <= Wire.available())  // if two bytes were received 
  {
    int temperature;
    byte msb, lsb;
    
    msb = Wire.read();  // receive high byte (full degrees)
    lsb = Wire.read();  // receive low byte (fraction degrees) 
    temperature = ((msb) << 4);  // MSB
    temperature |= (lsb >> 4);   // LSB
    
    temp = temperature*0.0625;
  }
}
