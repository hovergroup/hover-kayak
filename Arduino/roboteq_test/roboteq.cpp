#include "roboteq.h"

void Roboteq::initialize() {
  pinMode(7,OUTPUT);
  digitalWrite(7,HIGH);
  delay(1000);
}

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
  Serial3.println(mybuf);
}

void Roboteq::parseTemperature( int index, int stopIndex ) {
  if ( buffer[index]=='T' && buffer[index+1]=='=' ) {
    sscanf( &buffer[index], "T=%d:%d", &internal_temp, &heatsink_temp );
  } else {
    Serial.println("bad parse");
  }
}

void Roboteq::parseVoltage( int index, int stopIndex ) {
  if ( buffer[index]=='V' && buffer[index+1]=='=' ) {
    int v1, v2, v3;
    sscanf( &buffer[index], "V=%d:%d:%d", &v1, &v2, &v3 );
    internal_voltage = v1/10.0;
    battery_voltage = v2/10.0;
    five_voltage = v3/1000.0;
  } else {
    Serial.println("bad parse");
  }
}

void Roboteq::parseBatteryAmps( int index, int stopIndex ) {
  if ( buffer[index]=='B' && buffer[index+1]=='A' && buffer[index+2]=='=' ) {
    int temp_store;
    sscanf( &buffer[index], "BA=%d", &temp_store ); 
    battery_amps = temp_store/10.0;
  } else {
    Serial.println("bad parse");
  }
}

void Roboteq::parseMotorAmps( int index, int stopIndex ) {
  if ( buffer[index]=='A' && buffer[index+1]=='=' ) {
    int temp_store;
    sscanf( &buffer[index], "A=%d", &temp_store ); 
    motor_amps = temp_store/10.0;
  } else {
    Serial.println("bad parse");
  }
}

void Roboteq::parsePowerOutput( int index, int stopIndex ) {
  if ( buffer[index]=='P' && buffer[index+1]=='=' ) {
    sscanf( &buffer[index], "P=%d", &power_output ); 
  } else {
    Serial.println("bad parse");
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
  } else {
    Serial.println("bad parse");
  }
}

Roboteq::Roboteq(Stream& port) : _port(port)
{
  _port.setTimeout(ROBOTEQ_TIMEOUT);
  last_read_time = 0;
  read_state = 0;
  buffer_index=0;
}








// update fault flags
boolean Roboteq::updateFaultFlags() {
  _port.println("?FF");
   
  int bytesRead = readLine();
  if ( bytesRead == 0 ) return false;
  else if ( buffer[0]=='F' && buffer[1]=='F' && buffer[2]=='=' ) {
    // flags are stored in idividual bits
    byte faults = atoi(&buffer[3]);
    overheat = faults&0x01;
    overvoltage = faults&0x02;
    undervoltage = faults&0x04;
    short_circuit = faults&0x08;
    emergency_stop = faults&0x10;
    sepex_excitation_fault = faults&0x20;
    mosfet_failure = faults&0x40;
    startup_configuration_fault = faults&0x80;
    return true;
  } else return false;
}

// update status flags
boolean Roboteq::updateStatusFlags() {
  _port.println("?FS");
   
  int bytesRead = readLine();
  if ( bytesRead == 0 ) return false;
  else if ( buffer[0]=='F' && buffer[1]=='S' && buffer[2]=='=' ) {
    // flags are stored in individual bits
    byte faults = atoi(&buffer[3]);
    serial_mode = faults&0x01;
    pulse_mode = faults&0x02;
    analog_mode = faults&0x04;
    power_stage_off = faults&0x08;
    stall_detected = faults&0x10;
    at_limit = faults&0x20;
    // unused faults&0x40
    script_running = faults&0x80;
    return true;
  } else return false;
}

// read a single line
int Roboteq::readLine() {
  return _port.readBytesUntil( '\r', &buffer[0], 100 );
}

char* Roboteq::getFaultSummary() {
  *summary_string = '\0';
  if ( overheat ) strcat( summary_string, "overheat,");
  if ( overvoltage ) strcat( summary_string, "overvoltage,");
  if ( undervoltage ) strcat( summary_string, "undervoltage,");
  if ( undervoltage ) strcat( summary_string, "short_circuit,");
  if ( emergency_stop ) strcat( summary_string, "emergency_stop,");
  if ( sepex_excitation_fault ) strcat( summary_string, "sepex_excitation_fault,");
  if ( mosfet_failure ) strcat( summary_string, "mosfet_failure,");
  if ( startup_configuration_fault ) strcat( summary_string, "startup_configuration_fault,");
  
  int length = strlen(summary_string);
  if ( length>2 )
    summary_string[length-2]='\0';
  
  return summary_string;
}

char* Roboteq::getStatusSummary() {
  *summary_string = '\0';
  if ( serial_mode ) strcat( summary_string, "serial_mode,");
  if ( pulse_mode ) strcat( summary_string, "pulse_mode,");
  if ( analog_mode ) strcat( summary_string, "analog_mode,");
  if ( power_stage_off ) strcat( summary_string, "power_stage_off,");
  if ( stall_detected ) strcat( summary_string, "stall_detected,");
  if ( at_limit ) strcat( summary_string, "at_limit,");
  if ( script_running ) strcat( summary_string, "script_running,");
  
  int length = strlen(summary_string);
  if ( length>2 )
    summary_string[length-2]='\0';
  
  return summary_string;
}
