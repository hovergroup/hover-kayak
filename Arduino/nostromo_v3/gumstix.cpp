#include "gumstix.h"

GumstixSerial::GumstixSerial(Stream& port) : _port(port)
{
  thrust = 0;
  rudder = 0;
  commands_ready = false;
  last_command_time = 0;
  badParseCount = 0;
  motorCommandCount = 0;
  totalBadParseCount = 0;
  last_slow_publish = 0;
  last_fast_publish = 0;
  fastReportCount = 0;
  slowReportCount = 0;
}

void GumstixSerial::doWork() {
  // get new data from the serial port
  readBuffer();
  shiftBuffer( processBuffer() );
  if (millis() - last_command_time > 1000)
    commands_ready = false;
  else
    commands_ready = true;
    
  if (millis() - last_slow_publish > slow_publish_period) {
    publishSlow();
    last_slow_publish=millis(); 
  }
  if (millis() - last_fast_publish > fast_publish_period) {
    publishFast();
    last_fast_publish=millis();
  }
}

void GumstixSerial::readBuffer() {
  while ( _port.available()!=0 ) {
    buffer[buffer_index]=_port.read();
    buffer_index++;
  }
//  for ( int i=0; i<buffer_index; i++ ) {
//    Serial.print(buffer[i]);
//  }
//  if ( buffer_index!=0 ) {
//  Serial.println();
//  }
}

int GumstixSerial::findLine(int index) {
  for ( int i=index; i<buffer_index; i++ ) {
    if ( buffer[i]=='#' )
      return i;
  }
  return -1;
}

void GumstixSerial::shiftBuffer( int shift ) {
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

int GumstixSerial::processBuffer() {
  int bytesUsed=0;
  int stopIndex = findLine(bytesUsed);
  if ( stopIndex == -1 ) return bytesUsed;
  while ( bytesUsed < buffer_index ) {
    if ( bytesUsed > stopIndex ) {
      stopIndex = findLine( bytesUsed );
      if ( stopIndex == -1 ) return bytesUsed;
    }
    switch ( buffer[bytesUsed] ) {
      case 'M':
        parseMotorCommand( bytesUsed, stopIndex );
        bytesUsed = stopIndex;
        break;
      case 'R':
        parseRadioCommand(bytesUsed, stopIndex);
        bytesUsed = stopIndex;
        break;
    }
    bytesUsed++;
  }
  return bytesUsed;
}

void GumstixSerial::parseMotorCommand( int index, int stopIndex ) {
//  for ( int i=index; i<=stopIndex; i++ ) {
//    Serial.print(buffer[i]);
//  }
//  return;
  
  if ( buffer[index]=='M' && buffer[index+1]=='=' ) {
    sscanf( &buffer[index], "M=%d,%d", &thrust, &rudder );
    last_command_time = millis();
    motorCommandCount++;
  } else {
    badParseCount++;
  }
}

void GumstixSerial::parseRadioCommand(int index, int stopIndex) {
  if ( buffer[index]=='R' && buffer[index+1]=='=' ) {
    int cmd;
    sscanf( &buffer[index], "R=%d", &cmd);
    if (cmd)
      radio.powerFreewave();
    else
      radio.powerBullet();
  } else {
    badParseCount++;
  }
}

void GumstixSerial::rolloverCounters(float elapsed_seconds) {
  motorCommandRate = motorCommandCount / elapsed_seconds;
  badParseRate = badParseCount / elapsed_seconds;
  slowReportRate = slowReportCount / elapsed_seconds;
  fastReportRate = fastReportCount / elapsed_seconds;
  
  totalBadParseCount += badParseRate;

  motorCommandCount = 0;
  badParseCount = 0;
  fastReportCount = 0;
  slowReportCount = 0;
}

void GumstixSerial::publishSlow() {
  char message[100];
  sprintf( &message[0], "?S=%d,%d,%d,%d,%d,%d,%d,%d",
    (int) (roboteq.getBatteryVoltage()*10.0),
    (int) (roboteq.getBatteryAmps()*10.0),
    (int) (roboteq.getMotorAmps()*10.0), 
    (int) (tmp102.getTemp()*10.0),
    roboteq.getHeatsinkTemp(),
    roboteq.getInternalTemp(),
    battery.getThrustLimit(),
    radio.getRadioPower() ); // 0 = bullet, 1 = freewave
  _port.println(message);
  slowReportCount++;
}

void GumstixSerial::publishFast() {
  char message[20];
  sprintf( &message[0], "?M=%d,%d", roboteq.getPowerOutput(), azimuth.getCurrentAngle() );
  _port.println(message);
  fastReportCount++;
}
