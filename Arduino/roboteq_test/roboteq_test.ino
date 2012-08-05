#include "roboteq.h"

Roboteq my_driver = Roboteq( Serial2 );

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200);
}


int state = 0;
int delay_time = 3000;
int last_state_time = 0;

void loop() {
  if ( millis()-last_state_time > delay_time ) {
    switch ( state ) {
    case 0:
      
      break;  
    }
    
    last_state_time=millis();
    state++;
  }
}
