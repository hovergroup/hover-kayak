#include "gumstix_serial.h"

GUMSTIX_SERIAL::GUMSTIX_SERIAL(Stream& port) : _port(port)
{
  thrust = 0;
  rudder = 0;
  last_command_time = 0;
}

void GUMSTIX_SERIAL::initialize() {}

void GUMSTIX_SERIAL::doWork() {
  // get new data from the serial port
  readBuffer();
  shiftBuffer( processBuffer() );
}

void GUMSTIX_SERIAL::readBuffer() {
  while ( _port.available()!=0 ) {
    buffer[buffer_index]=_port.read();
    buffer_index++;
  }
}

int GUMSTIX_SERIAL::findLine(int index) {
  for ( int i=index; i<buffer_index; i++ ) {
    if ( buffer[i]=='\r' )
      return i;
  }
  return -1;
}

void GUMSTIX_SERIAL::shiftBuffer( int shift ) {
//  Serial.print("shifting ");
//  Serial.print( buffer_index-shift );
//  Serial.print(" bytes by ");
//  Serial.println( shift );
//  Serial.println("preshift: ");
//  
//  for ( int i=0; i<buffer_index; i++ ) {
//    if ( buffer[i]=='\r' ) Serial.print("#");
//    else Serial.print(buffer[i]);
//  }
//  Serial.println();
  
  if ( shift==0 || buffer_index==0 ) return;
  for ( int i=shift; i<buffer_index; i++ ) {
    buffer[i-shift] = buffer[i];
  }
  buffer_index-=shift;
  
//  Serial.println("postshift: ");
//  for ( int i=0; i<buffer_index; i++ ) {
//    if ( buffer[i]=='\r' ) Serial.print("#");
//    else Serial.print(buffer[i]);
//  }
//  Serial.println();
}

int GUMSTIX_SERIAL::processBuffer() {
  int bytesUsed=0;
  int stopIndex = findLine(bytesUsed);
  if ( stopIndex == -1 ) return bytesUsed;
  while ( bytesUsed < buffer_index ) {
    if ( bytesUsed > stopIndex ) {
      stopIndex = findLine( bytesUsed );
      if ( stopIndex == -1 ) return bytesUsed;
    }
    switch ( buffer[bytesUsed] ) {
      case 'V':
        parseMotorCommand( bytesUsed, stopIndex );
        bytesUsed = stopIndex;
        break;
    }
    bytesUsed++;
  }
  return bytesUsed;
}

void GUMSTIX_SERIAL::parseMotorCommand( int index, int stopIndex ) {
  if ( buffer[index]=='M' && buffer[index+1]=='=' ) {
    sscanf( &buffer[index], "M=%d,%d", thrust, rudder );
    last_command_time = millis();
  } else {
    Serial.println("bad parse");
  }
}

