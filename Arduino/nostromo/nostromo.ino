#include <Wire.h>
#include <Servo.h>

#include "configuration.h"
#include "motor.h"
#include "sbus.h"
#include "Sabertooth.h"
#include "gumstix_serial.h"

Motor motor = Motor();
SBUS sbus = SBUS();
GUMSTIX_SERIAL gumstix = GUMSTIX_SERIAL();


void setup() {
  Wire.begin();
  
  // turn on the battery
  pinMode(mainBattery5vRelayPin, OUTPUT);
  digitalWrite(mainBattery5vRelayPin, HIGH);
  pinMode(mainBattery12vRelayPin, OUTPUT);
  digitalWrite(mainBattery12vRelayPin, HIGH);
  pinMode(servoPowerRelayPin, OUTPUT);
  
  // open serial ports
  Serial.begin(115200);
  Serial1.begin(115200);
  Serial3.begin(9600);
  
  // sbus serial port needs special settings
  Serial2.begin(100000);
  UCSR2C = (1<<USBS0)|(3<<UCSZ00);
  UCSR2C |= B00110000;
  delay(500);
  
  // start the motor driver
  motor.initialize();
  delay(200);
}

int millivolts; // batrery voltage
const int publishPeriod = 100; // how often we upload data
unsigned long last_time2 = 0;
boolean use_rc;

void publishSensorReadings() {
  char output[200]; 
  sprintf( &output[0], "voltage=%d,pwm=%d:angle=%d", 
    millivolts, 
    motor.getCurrentPWM(),
    motor.getCurrentAngle());
  Serial1.println( output );
  Serial.println( output );
}

void loop() {
  delay(1);
  
  // check battery voltage
  millivolts = voltageDividerGain * analogRead( voltageDividerPinNumber );
  
  // limit motor speed based on battery voltage
  if ( millivolts < 11200 )
    motor.limitPWM(20);
  else if ( millivolts < 10600 )
    motor.limitPWM(0);
  else if ( millivolts > 11800 )
    motor.limitPWM(100);
  
  // comms iterate
  gumstix.doWork();
  
  // sbus iterate
  if ( sbus.doWork() ) {
    if ( sbus.failsafe_status == SBUS_SIGNAL_OK && sbus.channels[4] > 1000 ) {
      use_rc = true;
    } else {
      use_rc = false;
    }
  }
  
  if ( use_rc ) {
    // toggle channel used to enable motor
    int toggle_channel = sbus.channels[6];
    if ( toggle_channel < 1000 ) {
      // map thrust percentage
      int thrust_channel = constrain( sbus.channels[1], 368, 1650);
      int target_pwm = map(thrust_channel, 1630, 368, -100, 100);
      motor.setPWM(target_pwm);
      
      // map rudder angle
      int azimuth_channel = constrain( sbus.channels[0], 364, 1686);
      int target_angle = map( azimuth_channel , 364, 1686, -60, 60);
      motor.setAngle( target_angle );
    } else {
      motor.setPWM(0);
    }
  } else {
    // get commands from gumstix serial
    if ( gumstix.new_motor_command ) {
      gumstix.new_motor_command = false;
      motor.setPWM(gumstix.motor_thrust);
      motor.setAngle(gumstix.motor_rudder);
    }
    // if haven't received command recently, stop
    if ( millis() - gumstix.last_motor_command_time > 1000 ) {
      motor.setPWM(0);
//      Serial.println("Motor command timeout.");
    }
  }
    
  // run the motor driver
  boolean enabled = motor.doWork();
  // turn servo on or off
  if ( enabled )
    digitalWrite(servoPowerRelayPin, HIGH);
  else
    digitalWrite(servoPowerRelayPin, LOW);
  
  // publish sensor readings
  if ( millis() - last_time2 > publishPeriod ) {
    publishSensorReadings();
    last_time2 = millis(); 
  }
  
}
