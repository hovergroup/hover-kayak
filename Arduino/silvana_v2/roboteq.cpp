#include "roboteq.h"

boolean Roboteq::doWork() { 
  if ( millis()-last_read_time > ROBOTEQ_READ_UPDATE ) {
    last_read_time = millis();
    switch ( read_state ) {
    case 0:
      _port.println("?V");
      break;
    case 1:
      _port.println("?T");
      break;
    case 2:
      _port.println("?A");
      break;
    case 3:
      _port.println("?BA");
      break;
    case 4:
      _port.println("?DI 1");
      read_state=-1;
      break;
    }
    _port.println("?P");
    read_state++;
    sendSpeed( current_velocity );
  }
  
  readBuffer();
  shiftBuffer( processBuffer() );
}

void Roboteq::shiftBuffer( int shift ) {
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

void Roboteq::readBuffer() {
  while ( _port.available()!=0 ) {
    buffer[buffer_index]=_port.read();
    buffer_index++;
  }
}

int Roboteq::processBuffer() {
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
        parseVoltage( bytesUsed, stopIndex );
        bytesUsed = stopIndex;
        break;
      case 'T':
        parseTemperature( bytesUsed, stopIndex );
        bytesUsed = stopIndex;
        break;
      case 'P':
        parsePowerOutput( bytesUsed, stopIndex );
        bytesUsed = stopIndex;
        break;
      case 'B':
        parseBatteryAmps( bytesUsed, stopIndex );
        bytesUsed = stopIndex;
        break;
      case 'A':
        parseMotorAmps( bytesUsed, stopIndex );
        bytesUsed = stopIndex;
        break;
      case 'D':
        parseStopSwitch( bytesUsed, stopIndex );
        bytesUsed = stopIndex;
        break;
      case '?':
        bytesUsed = stopIndex;
        break;
    }
    bytesUsed++;
  }
  return bytesUsed;
}

int Roboteq::findLine(int index) {
  for ( int i=index; i<buffer_index; i++ ) {
    if ( buffer[i]=='\r' )
      return i;
  }
  return -1;
}

void Roboteq::setPower( int velocity ) {
  current_velocity = constrain(velocity,-1000,1000);
}

void Roboteq::sendSpeed( int velocity ) {
  char mybuf[10];
  sprintf(mybuf,"!G %d",velocity);
  _port.println(mybuf);
  speedCount++;
}

void Roboteq::setLights(boolean onoff) {
  lights_state = onoff;
  if (onoff)
    _port.println("!D0 1");
  else
    _port.println("!D1 1");
}

void Roboteq::toggleLights() {
  setLights(!lights_state);
}

void Roboteq::parseTemperature( int index, int stopIndex ) {
  if ( buffer[index]=='T' && buffer[index+1]=='=' ) {
    sscanf( &buffer[index], "T=%d:%d", &internal_temp, &heatsink_temp );
    tempCount++;
  } else {
    badParseCount++;
  }
}

void Roboteq::parseVoltage( int index, int stopIndex ) {
  if ( buffer[index]=='V' && buffer[index+1]=='=' ) {
    int v1, v2, v3;
    sscanf( &buffer[index], "V=%d:%d:%d", &v1, &v2, &v3 );
    internal_voltage = v1/10.0;
    battery_voltage = .05*v2/10.0 + .95*battery_voltage;
    five_voltage = v3/1000.0;
    voltageCount++;
  } else {
    badParseCount++;
  }
}

void Roboteq::parseBatteryAmps( int index, int stopIndex ) {
  if ( buffer[index]=='B' && buffer[index+1]=='A' && buffer[index+2]=='=' ) {
    int temp_store;
    sscanf( &buffer[index], "BA=%d", &temp_store ); 
    battery_amps = temp_store/10.0;
    batteryAmpCount++;
  } else {
    badParseCount++;
  }
}

void Roboteq::parseMotorAmps( int index, int stopIndex ) {
  if ( buffer[index]=='A' && buffer[index+1]=='=' ) {
    int temp_store;
    sscanf( &buffer[index], "A=%d", &temp_store ); 
    motor_amps = temp_store/10.0;
    motorAmpCount++;
  } else {
    badParseCount++;
  }
}

void Roboteq::parsePowerOutput( int index, int stopIndex ) {
  if ( buffer[index]=='P' && buffer[index+1]=='=' ) {
    sscanf( &buffer[index], "P=%d", &power_output ); 
    powerOutCount++;
  } else {
    badParseCount++;
  }
}

void Roboteq::parseStopSwitch( int index, int stopIndex ) {
  if ( buffer[index]=='D' && buffer[index+1]=='I' && buffer[index+2]=='=' ) {
    int temp;
    sscanf( &buffer[index], "DI=%d", &temp ); 
    if ( temp==1 )
      stop_switch = true;
    else
      stop_switch = false;
    stopSwitchCount++;
  } else {
    badParseCount++;
  }
}

void Roboteq::rolloverCounters(float elapsed_seconds) {
  speedRate = speedCount / elapsed_seconds;
  voltageRate = voltageCount / elapsed_seconds;
  tempRate = tempCount / elapsed_seconds;
  motorAmpRate = motorAmpCount / elapsed_seconds;
  batteryAmpRate = batteryAmpCount / elapsed_seconds;
  stopSwitchRate = stopSwitchCount / elapsed_seconds;
  powerOutRate = powerOutCount / elapsed_seconds;
  badParseRate = badParseCount / elapsed_seconds;
  
  total_bad_parse_count += badParseRate;

  speedCount = 0;
  voltageCount = 0;
  tempCount = 0;
  motorAmpCount = 0;
  batteryAmpCount = 0;
  stopSwitchCount = 0;
  powerOutCount = 0;
  badParseCount = 0;
}

Roboteq::Roboteq(Stream& port) : _port(port)
{
  last_read_time = 0;
  read_state = 0;
  buffer_index=0;
  battery_voltage = 0;
  total_bad_parse_count = 0;
  
  stop_switch = true;
  
  speedCount = 0;
  voltageCount = 0;
  tempCount = 0;
  motorAmpCount = 0;
  batteryAmpCount = 0;
  stopSwitchCount = 0;
  powerOutCount = 0;
  badParseCount = 0;
}

