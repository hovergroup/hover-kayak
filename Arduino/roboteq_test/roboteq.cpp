#include "roboteq.h"

boolean Roboteq::doWork() { 
  if ( millis()-last_read_time > ROBOTEQ_READ_UPDATE ) {
    last_read_time = millis();
    switch ( read_state ) {
    case 0:
      updateFaultFlags();
      read_state++;
      break;
    case 1:
      updateStatusFlags();
      read_state++;
      break;
    case 2:
      updateTemperature();
      read_state++;
      break;
    case 3:
      updateVoltage();
      read_state=0;
      break;
    }
  }
}

Roboteq::Roboteq(Stream& port) : _port(port)
{
  _port.setTimeout(ROBOTEQ_TIMEOUT);
  last_read_time = 0;
  read_state = 0;
}

// turn on emergency stop
void Roboteq::setEmergencyStop() {
  _port.println("!EX");
}

// turn off emergency stop
void Roboteq::resetEmergencyStop() {
  _port.println("!MG");
}

// set output speed, limits determined by max rpm config
void Roboteq::setOutput( int output ) { // -1000 to 1000
  char buffer [10];
  sprintf(buffer, "!G %d", output);
  _port.println(buffer);
}

// set maximum acceleration in output/sec
void Roboteq::setAccleration( int acceleration ) {
  char buffer [15];
  sprintf(buffer, "!AC 1 %d", acceleration);
  _port.println(buffer);
}

// set maximum deceleration in output/sec
void Roboteq::setDeceleration( int deceleration ) {
  char buffer [15];
  sprintf(buffer, "!DS 1 %d", deceleration);
  _port.println(buffer);
}

// read motor current
float Roboteq::readMotorAmps() {
  _port.println("?A"); 
  
  int bytesRead = readLine();
  if ( bytesRead == 0 ) return -1;
  else if ( buffer[0]=='A' && buffer[1]=='=' ) return atof(&buffer[2])/10.0;
  else return -1;
}

// read battery current
float Roboteq::readBatteryAmps() {
  _port.println("?BA");
  
  int bytesRead = readLine();
  if ( bytesRead == 0 ) return -1;
  else if ( buffer[0]=='B' && buffer[1]=='A' && buffer[2]=='=' ) return atof(&buffer[3])/10.0;
  else return -1;
}

// read a single digital input line
boolean Roboteq::readSingleDigitalInput( int input_number ) {
  char buffer[10];
  sprintf(buffer,"?DI %d", input_number);
  _port.println(buffer);
  
  int bytesRead = readLine();
  if ( bytesRead == 0 ) return -1;
  else if ( buffer[0]=='D' && buffer[1]=='I' && buffer[2]=='=' ) return atoi(&buffer[3]);
  else return -1;
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

// read temperatures
boolean Roboteq::updateTemperature() {
  _port.println("?T");
  
  int bytesRead = readLine();
  if ( bytesRead == 0 ) return false;
  else if ( buffer[0]=='T' && buffer[1]=='=' ) {
    char mybuf[10];
    int buffix = 0;
    int state = 0;
    // state 0, getting internal temp
    // state 1, found :
    // state 2, getting heatsink temp
    for ( int i=2; i<bytesRead; i++ ) {
      switch( buffer[i] ) {
      case ':':
        mybuf[buffix]=0x00;
        state++;
        break;
      default:
        mybuf[buffix]=buffer[i];
        buffix++;
      }
      if ( state==1 ) {
        internal_temp = atoi(mybuf);
        state++;
        buffix=0;
      }
    }
    mybuf[buffix]=0x00;
    heatsink_temp = atoi(mybuf);
    return true;
  }
  return false;
}

boolean Roboteq::updateVoltage() {
  _port.println("?V");
  
  int bytesRead = readLine();
  if ( bytesRead == 0 ) return false;
  else if ( buffer[0]=='T' && buffer[1]=='=' ) {
    char mybuf[10];
    int buffix = 0;
    int state = 0;
    // state 0 - getting internal voltage
    // state 1 - got first :
    // state 2 - getting battery voltage
    // state 3 - got second :
    // state 4 - getting 5v supply voltage
    for ( int i=2; i<bytesRead; i++ ) {
      switch( buffer[i] ) {
      case ':':
        mybuf[buffix]=0x00;
        state++;
        break;
      default:
        mybuf[buffix]=buffer[i];
        buffix++;
      }
      switch ( state ) {
      case 1:
        internal_voltage = atoi(mybuf)/10.0;
        state++;
        buffix=0;
        break; 
      case 3:
        battery_voltage = atoi(mybuf)/10.0;
        state++;
        buffix=0;
        break;
      default:
        break;
      }
    }
    mybuf[buffix]=0x00;
    five_voltage = atoi(mybuf)/1000.0;
    return true;
  }
  return false;
}

// read a single line
int Roboteq::readLine() {
  _port.readBytesUntil( '\r', &buffer[0], 100 );
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
