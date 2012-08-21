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
  for ( int i=0; i<1000/25; i++ ) {
  delay(25);
  my_driver.doWork();
  }
  
  Serial.println( my_driver.getHeatsinkTemp() );
  Serial.println( my_driver.getBatteryVoltage() );
//  switch ( state ) {
//  case 0:
//    my_driver.setPower(1000);
//    break;
//  case 1:
//    my_driver.setPower(-1000);
//    break;
//  case 2:
//    my_driver.setPower(0);
//    state =-1;
//    break;
//  }
//  state++;
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
