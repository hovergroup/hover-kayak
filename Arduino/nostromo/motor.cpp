#include "motor.h"

Motor::Motor() { 
  last_time = 0;
  current_percent = 0;
  target_percent = 0;
  enabled = true;
  limit_percent = 100;
}

void Motor::limitPWM( int percentage ) {
  if ( percentage > 100 || percentage < 0 )
    return;
  limit_percent = percentage; 
  target_percent = constrain(percentage, limit_percent, limit_percent);
}

boolean Motor::doWork() {
  if ( digitalRead(thrusterEStopPin) || limit_percent == 0 )
    enabled = true;
  else {
    enabled = false;
    current_percent = 0;
  }
  
  unsigned long time_elapsed = millis() - last_time;
  last_time = millis();
 
  if ( current_percent != target_percent ) {
    double max_deviation = (time_elapsed*thrusterSlewLimit/1000.0);
//    Serial.print(max_deviation);
//    Serial.print(" ");
    
    if ( (abs(current_percent) < abs(target_percent)-max_deviation) ||
         (abs(current_percent) > abs(target_percent)+max_deviation) ||
         (current_percent > 0 && target_percent < 0) ||
         (current_percent < 0 && target_percent > 0) ) {
      if ( target_percent > current_percent )
        current_percent += max_deviation;
      else
        current_percent -= max_deviation;
    } else
      current_percent = target_percent;
    
  }
    outputPWM( (int) current_percent );
    return enabled;
}

void Motor::initialize() {    
  azimuth_servo.attach(azimuthServoPinNumber);
  azimuth_servo.writeMicroseconds(1500);
 
  pinMode(thrusterEStopPin, OUTPUT);
  
  ST.setTimeout(950);
}

void Motor::outputPWM( int percent ) {
  if ( enabled ) {
    int output = map( percent, -100, 100, -127, 127 );
    ST.motor(1,output);
    Serial.print("st: ");
    Serial.println(output);
    
  } else {
    unsigned char output = 127;
    ST.motor(1,0);
  }
}

void Motor::setPWM( int percentage ) {
  target_percent = constrain(percentage, -limit_percent, limit_percent);
  
//  if ( abs(current_percent) > abs(target_percent) ) {
//    current_percent = target_percent;
//    outputPWM( target_percent );
//  }
}

void Motor::setAngle( int angle ) {
  int set_angle = constrain( angle, -60, 60);
//  int microseconds = map(set_angle, -60, 60, azimuthLeft60, azimuthRight60);
  int microseconds = map(set_angle, -60, 60, azimuthCenter-azimuth60Range, azimuthCenter+azimuth60Range);
//  Serial.println(microseconds);
  azimuth_servo.writeMicroseconds( microseconds ); 
  //azimuth_servo.write(angle);
}
