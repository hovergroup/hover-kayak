#include "motor.h"

Motor::Motor() { 
  last_time = 0;
  current_percent = 0;
  target_percent = 0;
  batt_voltage = 1;
  enabled = true;
}

void Motor::setVoltage( double voltage ) {
  if ( voltage == 0 )
    batt_voltage = 1;
  else
    batt_voltage = voltage;
}

void Motor::setEnable( boolean onoff ) {
  if ( !onoff ) {
    outputPWM(0);
    enabled = false;
  } else
    enabled = true;
}

double Motor::readCurrent() {
  int pini = analogRead(motorCurrentSensePinNumber);
  int pinv = map(pini, 0, 1023, 0, 5000);
  return (pinv-base_current)/66.0;
}

void Motor::doWork() {
  unsigned long time_elapsed = millis() - last_time;
  last_time = millis();
 
  if ( current_percent != target_percent ) {
    double max_deviation = (time_elapsed*thrusterSlewLimit/1000.0);
//    Serial.print(max_deviation);
//    Serial.print(" ");
    
    if ( current_percent < target_percent-max_deviation )
      current_percent += max_deviation;
    else
      current_percent = target_percent;
    
    outputPWM( (int) current_percent );
  }
}

void Motor::initialize() {
  pinMode(motorPWMPinNumber, OUTPUT);
  pinMode(motorDirectionPinNumber, OUTPUT);
  pinMode(motorResetPinNumber, OUTPUT);
  
  digitalWrite(motorResetPinNumber, HIGH);
  digitalWrite(motorPWMPinNumber, LOW);
  
  setDirection( true );
  
  azimuth_servo.attach(azimuthServoPinNumber);
  azimuth_servo.writeMicroseconds(1500);
  
  int pini = analogRead( motorCurrentSensePinNumber );
  base_current = map(pini, 0, 1023, 0, 5000);
}

void Motor::setDirection( boolean forward ) {
  if ( motorDirectionInverted )
    forward = !forward;
  if ( forward )
    digitalWrite( motorDirectionPinNumber, HIGH );
  else
    digitalWrite( motorDirectionPinNumber, LOW );
}

void Motor::outputPWM( int percent ) {
  if ( enabled ) {
    int output = map( percent, 0, 100, 0, 255 );
    int new_output = constrain( (int) output * 12.0/batt_voltage, 0, 255 );
    analogWrite( motorPWMPinNumber, new_output );
//    Serial.print("wrote pwm: ");
//    Serial.println(new_output);
  } else
    Serial.println("motor disabled");
}

void Motor::setPWM( int percentage ) {
  target_percent = constrain(percentage, 0, 100);
  
  if ( current_percent > target_percent ) {
    current_percent = target_percent;
    outputPWM( target_percent );
  }
//  Serial.print(percentage);
//  Serial.print(" ");
//  Serial.print(output);
//  Serial.print(" ");
//  Serial.println(new_output);
}

void Motor::setAngle( int angle ) {
  int microseconds = map(angle, -90, 90, 900, 2100);
  Serial.println(microseconds);
  azimuth_servo.writeMicroseconds( microseconds ); 
}
