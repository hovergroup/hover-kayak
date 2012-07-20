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
  
  pinMode(mainBattery5vRelayPin, OUTPUT);
  digitalWrite(mainBattery5vRelayPin, HIGH);
  pinMode(mainBattery12vRelayPin, OUTPUT);
  digitalWrite(mainBattery12vRelayPin, HIGH);
  pinMode(servoPowerRelayPin, OUTPUT);
  
  Serial.begin(115200);
  Serial1.begin(115200);
  Serial3.begin(9600);
  
  Serial2.begin(100000);
  UCSR2C = (1<<USBS0)|(3<<UCSZ00);
  UCSR2C |= B00110000;
  delay(500);
  
  motor.initialize();
  delay(200);
}

int millivolts;

void publishSensorReadings() {
  char output[200]; 
  sprintf( &output[0], "voltage=%d", millivolts );
  Serial1.println( output );
  Serial.println( output );
}

const int publishPeriod = 500;
unsigned long last_time = 0, last_time2 = 0;
int something = 1;
boolean use_rc;

void loop() {
  delay(1);
  
  millivolts = voltageDividerGain * analogRead( voltageDividerPinNumber );
  
  if ( millivolts < 11200 )
    motor.limitPWM(20);
  else if ( millivolts < 10600 )
    motor.limitPWM(0);
  else if ( millivolts > 11800 )
    motor.limitPWM(100);
  
  gumstix.doWork();
  
  if ( sbus.doWork() ) {
    if ( sbus.failsafe_status == SBUS_SIGNAL_OK && sbus.channels[4] > 1000 ) {
      use_rc = true;
    } else {
      use_rc = false;
    }
  }
  
  if ( use_rc ) {
    int toggle_channel = sbus.channels[6];
    if ( toggle_channel < 1000 ) {
      int thrust_channel = constrain( sbus.channels[1], 368, 1650);
      int target_pwm = map(thrust_channel, 1630, 368, -100, 100);
      motor.setPWM(target_pwm);
      //      Serial.print("percent: ");
      //      Serial.println(target_pwm);
      
      int azimuth_channel = constrain( sbus.channels[0], 364, 1686);
      int target_angle = map( azimuth_channel , 364, 1686, -60, 60);
      motor.setAngle( target_angle );
    } else {
      motor.setPWM(0);
    }
  } else {
    if ( gumstix.new_motor_command ) {
      gumstix.new_motor_command = false;
      motor.setPWM(gumstix.motor_thrust);
      motor.setAngle(gumstix.motor_rudder);
//    Serial.print("thrust: ");
//    Serial.print(gumstix.motor_thrust);
//    Serial.print("  rudder: ");
//    Serial.println(gumstix.motor_rudder);
    }
    if ( millis() - gumstix.last_motor_command_time > 1000 ) {
      motor.setPWM(0);
//      Serial.println("Motor command timeout.");
    }
  }
    
  boolean enabled = motor.doWork();
  if ( enabled )
    digitalWrite(servoPowerRelayPin, HIGH);
  else
    digitalWrite(servoPowerRelayPin, LOW);
  
  if ( millis() - last_time2 > publishPeriod ) {
    publishSensorReadings();
    last_time2 = millis(); 
  }
  
}
