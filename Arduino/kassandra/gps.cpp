#include "gps.h"

GPS::GPS() {
  buffer_index=0;
  startIndex = 0;
}

void GPS::initialize() {
 GPS_SERIAL.begin(38400); 
}

void GPS::doWork() {
//  Serial.print("reading gps: ");
//  Serial.println( GPS_SERIAL.available() );
  while ( GPS_SERIAL.available() > 0 ) {
    buffer[buffer_index] = GPS_SERIAL.read();
    
    if ( buffer[buffer_index] == '$' )
      startIndex = buffer_index;    
    else if ( buffer[buffer_index] == 0x0D ) {
      forwardGPS( buffer_index );
    }
    
    buffer_index++;
  }
}

void GPS::forwardGPS( int stopIndex ) {
//  Serial1.print("forwarded: ");
  for ( int i=startIndex; i<stopIndex; i++ ) {
    Serial1.print( buffer[i] );
  }
  Serial1.println();
  
  buffer_index = 0;
}
