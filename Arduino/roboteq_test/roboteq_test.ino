#include "roboteq.h"

Roboteq my_driver = Roboteq( Serial3 );

void setup() {
  Serial.begin(115200);
  Serial3.begin(115200);
  
  my_driver.initialize();
  
}


int state = 0;
int delay_time = 3000;
int last_state_time = 0;

void loop() {
  for ( int i=0; i<500/25; i++ ) {
  delay(25);
  my_driver.doWork();
  }
  Serial.print("battery: ");
  Serial.println( my_driver.getBatteryVoltage() );
  Serial.print("temp: " );
  Serial.println( my_driver.getHeatsinkTemp() );
//  if ( millis()-last_state_time > delay_time ) {
//    switch ( state ) {
//    case 0:
//      
//      break;  
//    }
//    
//    last_state_time=millis();
//    state++;
//  }
}
