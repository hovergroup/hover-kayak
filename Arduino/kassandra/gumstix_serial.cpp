#include "gumstix_serial.h"

GUMSTIX_SERIAL::GUMSTIX_SERIAL() {
  index = 0; 
  new_motor_command = false;
  compass_calibration_signal = false;
  motor_thrust = 0;
  motor_rudder = 0;
  last_motor_command_time = 0;
}

void GUMSTIX_SERIAL::initialize() {
  Serial1.begin(115200);
}

void GUMSTIX_SERIAL::doWork() {
  // get new data from the serial port
  readBuffer();
}

void GUMSTIX_SERIAL::readBuffer() {
  while ( Serial1.available() ) {
    buffer[index] = Serial1.read();
    Serial.print(buffer[index]);
    
    if ( buffer[index] == '\n' ) {
//      Serial.println("parsing");
      if ( index >= 3 )
        parseBuffer();
      index = 0;
    } else
      index++;
  }
  
  if ( index > 200 )
    index = 0;
}

void GUMSTIX_SERIAL::parseBuffer() {
  for ( int i=0; i<=index-3; i++ ) {
    if ( buffer[i]=='m' && buffer[i+1]=='t' && buffer[i+2]=='r' ) {
      parseMotorCommand(i);
      break;
    } else if ( buffer[i]=='c' && buffer[i+1]=='a' && buffer[i+2]=='l' ) {
//      Serial.println("got calibration signal");
      compass_calibration_signal = true;
      break;
    }
  }
}

void GUMSTIX_SERIAL::parseMotorCommand(int startIndex) {
  last_integer_end = startIndex+4;
  motor_thrust = parseNext();
  motor_rudder = parseNext();
  new_motor_command = true;
  last_motor_command_time = millis();
}

int GUMSTIX_SERIAL::parseNext() {
  int startPosition=-1, endPosition=-1;
  for ( int i=last_integer_end; i<=index; i++ ) {
    if ( buffer[i]=='=' )
      startPosition = i;
    else if ( buffer[i]==',' || buffer[i]=='\n' ) {
      endPosition = i;
      break;
    }
  }
//  Serial.println(startPosition);
//  Serial.println(endPosition);
  if ( endPosition==-1 || startPosition==-1 || endPosition-startPosition <= 1) {
    return 0;
  } else {
    char tmp [10];
    for ( int i=0; i<10; i++ ) {
      tmp[i] = 0x00;
    }
    memcpy( tmp, &buffer[startPosition+1],endPosition-startPosition-1 );
    last_integer_end = endPosition+1;
    return atoi(tmp);
  }
}
