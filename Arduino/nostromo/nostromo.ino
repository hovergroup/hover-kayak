#include <Wire.h>
#include <Servo.h>

#include "configuration.h"
#include "sbus.h"
#include "roboteq.h"
#include "azimuth.h"
#include "gumstix_serial.h"

Roboteq roboteq = Roboteq( Serial2 );
Azimuth azimuth = Azimuth();
SBUS sbus = SBUS(Serial3);
GUMSTIX_SERIAL gumstix = GUMSTIX_SERIAL();


void setup() {
  Wire.begin();
  
  // turn on the regulators
  pinMode(mainBattery5vRelayPin, OUTPUT);
  digitalWrite(mainBattery5vRelayPin, HIGH);
  pinMode(mainBattery12vRelayPin, OUTPUT);
  digitalWrite(mainBattery12vRelayPin, HIGH);
  pinMode(servoPowerRelayPin, OUTPUT);
  
  // open serial ports
  Serial.begin(115200);
  Serial1.begin(115200);
  Serial2.begin(115200);
  
  // sbus serial port needs special settings
  Serial3.begin(100000);
  UCSR3C = (1<<USBS0)|(3<<UCSZ00);
  UCSR3C |= B00110000;
  delay(500);
  
  // start the motor driver
  Serial.println("initializing drivers");
  azimuth.initialize();
  roboteq.initialize();
  
  delay(200);
  
  Serial.println("warming drivers");
  for ( int i=0; i<100; i++ ) {
    roboteq.doWork();
    sbus.doWork();
    gumstix.doWork();
    delay(25);
  }
  
  Serial.println("ready");
}

float battery_voltage = 0;
const int publishPeriod = 100; // how often we upload data
unsigned long last_time2 = 0;
boolean use_rc;

//void publishSensorReadings() {
//  char output[200]; 
//  sprintf( &output[0], "voltage=%d,pwm=%d,angle=%d", 
//    millivolts, 
//    motor.getCurrentPWM(),
//    motor.getCurrentAngle());
//  Serial1.println( output );
//  Serial.println( output );
//}

int thrust_limit = 1000;

void setThrust( int thrust ) {
  thrust = constrain(thrust, -thrust_limit, thrust_limit);
  int current_angle = azimuth.getCurrentAngle();
  if ( abs(current_angle) >= limitStartAngle ) {
    int turn_limit = (abs(current_angle)-limitStartAngle)*limitSlope + limitOffset;
    thrust = constrain( thrust, -1000+turn_limit, 1000-turn_limit );
  }
  roboteq.setPower(thrust);
}

void loop() {
  
  // check battery voltage
//  millivolts = voltageDividerGain * analogRead( voltageDividerPinNumber );
  
  // limit motor speed based on battery voltage
//  if ( millivolts < 11200 )
//    motor.limitPWM(20);
//  else if ( millivolts < 10600 )
//    motor.limitPWM(0);
//  else if ( millivolts > 11800 )
//    motor.limitPWM(100);
  
  // comms iterate
  gumstix.doWork();
  roboteq.doWork();
  
  battery_voltage = roboteq.getBatteryVoltage();
  if ( battery_voltage < 11.2 )
    thrust_limit = 200;
  else if ( battery_voltage < 10.6 )
    thrust_limit = 0;
  else if ( battery_voltage > 11.8 )
    thrust_limit = 1000;
  
  // sbus iterate
  if ( sbus.doWork() ) {
    if ( sbus.failsafe_status == SBUS_SIGNAL_OK && sbus.channels[4] > 1000 ) {
      use_rc = true;
    } else {
      use_rc = false;
    }
  }
  
  if ( use_rc ) {
//    Serial.println("using rc");
    // toggle channel used to enable motor
    int toggle_channel = sbus.channels[6];
    if ( toggle_channel < 1000 ) {
//      Serial.println("rc toggle on");
      // map thrust percentage
      int thrust_channel = constrain( sbus.channels[1], 368, 1650);
      int target_pwm = map(thrust_channel, 1630, 368, -1000, 1000);
      if ( abs(target_pwm) < 50 ) target_pwm = 0;
      setThrust(target_pwm);
      
      // map rudder angle
      int azimuth_channel = constrain( sbus.channels[0], 364, 1686);
      int target_angle = map( azimuth_channel , 364, 1686, -60, 60);
      azimuth.setAngle( target_angle );
    } else {
      setThrust(0);
      azimuth.setAngle( 0 );
    }
  } else {
    // get commands from gumstix serial
    if ( gumstix.new_motor_command ) {
      gumstix.new_motor_command = false;
      setThrust(gumstix.motor_thrust);
      azimuth.setAngle(gumstix.motor_rudder);
    }
    // if haven't received command recently, stop
    if ( millis() - gumstix.last_motor_command_time > 1000 ) {
      setThrust(0);
      azimuth.setAngle(0);
//      Serial.println("Motor command timeout.");
    }
  }
    
  // turn servo on or off
  if ( roboteq.getStopSwitch() )
    digitalWrite(servoPowerRelayPin, LOW);
  else
    digitalWrite(servoPowerRelayPin, HIGH);
  
  // publish sensor readings
//  if ( millis() - last_time2 > publishPeriod ) {
//    publishSensorReadings();
//    last_time2 = millis(); 
//  }
  
}
