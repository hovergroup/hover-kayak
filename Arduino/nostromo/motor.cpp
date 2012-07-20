#include "motor.h"

Motor::Motor() { 
  last_time = 0;
  current_percent = 0;
  target_percent = 0;
  enabled = true;
  limit_percent = 100;
  current_angle = 0;
}

// set a limit on the pwm
void Motor::limitPWM( int percentage ) {
  // check limit is within range
  if ( percentage > 100 || percentage < 0 )
    return;
  // set limit and constrain current target
  limit_percent = percentage; 
  target_percent = constrain(percentage, -limit_percent, limit_percent);
}

// iterate method
boolean Motor::doWork() {
  // check status of estop
  if ( digitalRead(thrusterEStopPin) || limit_percent == 0 )
    enabled = true;
  else {
    // immediate stop
    enabled = false;
    target_percent = 0;
    current_percent = 0;
  }
  
  // time elapsed since last iteration
  unsigned long time_elapsed = millis() - last_time;
  last_time = millis();
 
  if ( current_percent != target_percent ) {
    // calculate a max change based on time elapsed
    double max_deviation = (time_elapsed*thrusterSlewLimit/1000.0);
    
    
    if ( (abs(current_percent) < abs(target_percent)-max_deviation) || // less than the target
         (abs(current_percent) > abs(target_percent)+max_deviation) || // greater than the target
         (current_percent > 0 && target_percent < 0) || // signs are opposite
         (current_percent < 0 && target_percent > 0) ) { // signs are opposite
      if ( target_percent > current_percent )
        current_percent += max_deviation; // increase thrust
      else
        current_percent -= max_deviation; // decrease thrust
    } else
      current_percent = target_percent; // close, snap to target
    
  }
  outputPWM( (int) current_percent ); // output new percent
  return enabled;
}

void Motor::initialize() {    
  // setup servo and center
  azimuth_servo.attach(azimuthServoPinNumber);
  setAngle( 0 );
 
  // configure e-stop pin
  pinMode(thrusterEStopPin, OUTPUT);
  
  // timeout on syrendriver
  ST.setTimeout(950);
}

// send a pwm to motor
void Motor::outputPWM( int percent ) {
  if ( enabled ) {
    // map percentage f enabled
    int output = map( percent, -100, 100, -127, 127 );
    ST.motor(1,output);
    
  } else {
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
  // constrain agnel
  int set_angle = constrain( angle, -60, 60);
  current_angle = set_angle;
  // map angle to microseconds
  int microseconds = map(set_angle, -60, 60, azimuthCenter-azimuth60Range, azimuthCenter+azimuth60Range);
  // send to servo
  azimuth_servo.writeMicroseconds( microseconds ); 
}
