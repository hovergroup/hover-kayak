#include <Wire.h>
#include <Servo.h>

#include "configuration.h"
#include "sbus.h"
#include "roboteq.h"
#include "azimuth.h"
#include "gumstix_serial.h"
#include "TMP102.h"

Roboteq roboteq = Roboteq( Serial2 );
Azimuth azimuth = Azimuth();
SBUS sbus = SBUS(Serial3);
GUMSTIX_SERIAL gumstix = GUMSTIX_SERIAL(Serial1);
TMP102 temp = TMP102();


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

void publishVoltages() {
  char message[20];
  sprintf( &message[0], "?V=%d", (int) (roboteq.getBatteryVoltage()*10.0) );
  Serial1.println(message);
//  Serial.println(message);  
}

void publishActuators() {
  char message[20];
  sprintf( &message[0], "?M=%d,%d", roboteq.getPowerOutput(), azimuth.getCurrentAngle() );
  Serial1.println(message);
//  Serial.println(message);  
}

void publishCurrents() {
  char message[20];
  sprintf( &message[0], "?C=%d,%d", 
    (int) (roboteq.getBatteryAmps()*10.0), 
    (int) (roboteq.getMotorAmps()*10.0) );
  Serial1.println(message);
//  Serial.println(message);  
}

void publishTemps() {
  char message[20];
  sprintf( &message[0], "?T=%d,%d,%d", 
    (int) (temp.getTemp()*10.0), 
    roboteq.getHeatsinkTemp(), 
    roboteq.getInternalTemp() );
  Serial1.println(message);
//  Serial.println(message);  
}

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

const int actuator_publish_period = 50;
const int general_publish_period = 500;
unsigned long actuator_publish_time=0, general_publish_time=0, last_command_fetch=0;
boolean use_rc;

void loop() {
  gumstix.doWork();
  roboteq.doWork();
  
  if ( millis()-actuator_publish_time > actuator_publish_period ) {
    publishActuators();
    actuator_publish_time = millis();
  }
  if ( millis()-general_publish_time > general_publish_period ) {
    temp.updateReading();
    publishVoltages();
    publishCurrents();
    publishTemps();
    general_publish_time = millis();
  }
  
  // comms iterate
  gumstix.doWork();
  roboteq.doWork();
  sbus.doWork();
  
  float battery_voltage = roboteq.getBatteryVoltage();
  if ( battery_voltage < 11.0 )
    thrust_limit = 200;
  else if ( battery_voltage < 10.6 )
    thrust_limit = 0;
  else if ( battery_voltage > 11.6 )
    thrust_limit = 1000;
  
  if ( use_rc ) {
    if ( !sbus.getSerialEnable() ) { // get commands from RC
      int target_pwm = sbus.getThrust();
      if ( abs(target_pwm) < 50 ) target_pwm = 0;
      setThrust(target_pwm);
      
      azimuth.setAngle( sbus.getRudder() );
    } else {
      setThrust(0);
      azimuth.setAngle( 0 );
    }
  } else {
    // get commands from gumstix serial
    if ( gumstix.getLastCommandTime() != last_command_fetch ) {
      last_command_fetch = gumstix.getLastCommandTime();
      setThrust( gumstix.getThrustCommand() );
      azimuth.setAngle( gumstix.getRudderCommand() );
    } else if ( millis() - gumstix.getLastCommandTime() > 1000 ) {
      setThrust(0);
      azimuth.setAngle(0);
    }
  }
    
  // turn servo on or off
  if ( roboteq.getStopSwitch() )
    digitalWrite(servoPowerRelayPin, LOW);
  else
    digitalWrite(servoPowerRelayPin, HIGH);
}
