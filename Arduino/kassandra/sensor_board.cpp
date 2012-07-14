#include "sensor_board.h"

float Temp::getTemp() {
  int pini = analogRead(temperatureAnalogPin);
  int pinv = map(pini, 0, 1023, 0, 5000);
  return (float)pinv * gain/1000 + offset;
}

float Humidity::getHumidity() {
  int pini = analogRead(humidityAnalogPin);
  int pinv = map(pini, 0, 1023, 0, 5000);
  return ( ((float)pinv)/1000 - .958 )/.0307;
}

Gyro::Gyro(int gyro_pin) {
  pin = gyro_pin;
}

void Gyro::initialize() {
  int pinv, pini;
  int numi = 1000;
  float pinsum = 0.0;
  for (int i=0;i<numi;i++){
    pini = analogRead(pin);  // read analog voltage output
    delay(2);
    pinv = map(pini,0,1023,0,5000); // convert input to mV
    pinsum += (float) pinv;
  }
  offset = (int)(pinsum / ((float) numi)); // set the offset
}

float Gyro::getOmega() {
  int pini = analogRead(pin);
  int pinv = map(pini,0,1023,0,5000); // convert input to mV
  float w = (float)(pinv - offset)/25.0; // comvert mV to deg/sec
  return -w;
}

Compass::Compass() {
  inCalibrationMode = false;
  compassAddress = 0x32 >> 1;
}

// First rotate about X OR Y, then rotate about Z
// two rotations total
void Compass::startCalibration() {
  Wire.beginTransmission(compassAddress);
  Wire.write(0x71);
  Wire.endTransmission();
  
  inCalibrationMode = true;
}

void Compass::stopCalibration() {
  Wire.beginTransmission(compassAddress);
  Wire.write(0x7E);
  Wire.endTransmission();
  
  inCalibrationMode = false;
}

void Compass::updateHPR() {
  if ( !inCalibrationMode ) {
    // step 1: instruct sensor to read echoes 
    Wire.beginTransmission(compassAddress);  // transmit to device
    // the address specified in the datasheet is 66 (0x42) 
    // but i2c adressing uses the high 7 bits so it's 33 
    Wire.write(0x50);  // Send a "Post Heading Data" (0x50) command to the HMC6343  
    Wire.endTransmission();  // stop transmitting 
    
    // step 2: wait for readings to happen 
    delay(1);  // datasheet suggests at least 1 ms 
    
    // step 3: request reading from sensor 
    Wire.requestFrom(compassAddress, 6);  // request 6 bytes from slave device #33 
    
    // step 4: receive reading from sensor 
    if(6 <= Wire.available()){     // if six bytes were received
      for(int i = 0; i<6; i++) {
        responseBytes[i] = Wire.read();			
      }
    }
    heading = ((int)(responseBytes[0]<<8 | responseBytes[1]))/10.0;  // heading MSB and LSB
    pitch    = ((int)(responseBytes[2]<<8 | responseBytes[3]))/10.0;  // pitch MSB and LSB
    roll    = ((int)(responseBytes[4]<<8 | responseBytes[5]))/10.0;  // roll MSB and LSB
  }
  
  heading += compassOffset;
  if ( heading > 360 )
    heading -= 360;
  else if ( heading < 0 )
    heading += 360;
}

void Compass::updateAcc(){
  // step 1: instruct sensor to read echoes 
  Wire.beginTransmission(compassAddress);  // transmit to device
  // the address specified in the datasheet is 66 (0x42) 
  // but i2c adressing uses the high 7 bits so it's 33 
  Wire.write(0x40);         // Send a "Post Acceleration Data" (0x40) command to the HMC6343  
  Wire.endTransmission();  // stop transmitting 
  
  // step 2: wait for readings to happen 
  delay(1);               // datasheet suggests at least 1 ms 
  
  // step 3: request reading from sensor 
  Wire.requestFrom(compassAddress, 6);  // request 6 bytes from slave device #33 
  
  // step 4: receive reading from sensor 
  if(6 <= Wire.available()){     // if six bytes were received
    for(int i = 0; i<6; i++) {
      responseBytes[i] = Wire.read();
    }
  }
  acc_x = ((int)(responseBytes[0]<<8 | responseBytes[1]))/10.0;  // Ax MSB and LSB
  acc_y   = ((int)(responseBytes[2]<<8 | responseBytes[3]))/10.0;  // Ay MSB and LSB
  acc_z   = ((int)(responseBytes[4]<<8 | responseBytes[5]))/10.0;  // Az MSB and LSB
}



