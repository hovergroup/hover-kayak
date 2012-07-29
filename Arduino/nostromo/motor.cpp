#include "motor.h"

Motor::Motor() { 
  last_time = 0;
  current_percent = 0;
  target_percent = 0;
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
  target_percent = constrain(target_percent, -limit_percent, limit_percent);
}

// iterate method
boolean Motor::doWork() {
  // check status of estop
  if ( !digitalRead(thrusterEStopPin) ) {
    // immediate stop
    snapToZero();
    // update this to avoid sudden start when e-stop released
    last_time = millis();
    // return not enabled
    return false;
  }
  
  // time elapsed since last iteration
  unsigned long time_elapsed = millis() - last_time;
  last_time = millis();
 
  // slew rate implemenation
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
  
  if ( limit_percent == 0 )
    return false; // return not enabled for servo shutoff
  else
    return true; // return enabled
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
  // constrain to appropriate range
  percent = constrain(percent, -100, 100);
  // apply turn limiting
  percent = turnLimit( percent );
  
  // map into output range and send
  int output = map( percent, -100, 100, -127, 127 );
  ST.motor(1,output);
  current_output = percent;
}

int Motor::turnLimit( int percent ) {
  if ( abs(current_angle) >= limitStartAngle ) {
    int turn_limit = (abs(current_angle)-limitStartAngle)*limitSlope + limitOffset;
    return constrain( percent, -turn_limit, turn_limit );
  }
}

void Motor::snapToZero() {
  target_percent = 0;
  current_percent = 0;
  ST.motor(1,0);
  current_output = 0;
}

void Motor::setPWM( int percentage ) {
  target_percent = constrain(percentage, -limit_percent, limit_percent);
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
