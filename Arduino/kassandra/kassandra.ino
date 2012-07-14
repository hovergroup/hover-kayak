#include <Wire.h>
#include <Servo.h>

#include "configuration.h"
#include "power.h"
#include "sensor_board.h"
#include "gps.h"
#include "motor.h"
#include "gumstix_serial.h"

Compass my_compass = Compass();
Power my_power = Power();
Gyro my_gyro = Gyro( gyroAnalogPin );
Temp my_temp = Temp();
Humidity my_humidity = Humidity();
Motor motor = Motor();
GPS gps = GPS();

GUMSTIX_SERIAL gumstix = GUMSTIX_SERIAL();

void setup() {
  Wire.begin();
  
  my_gyro.initialize();
  my_power.set5V( true );
  delay(200);
  my_power.setAux( false );
  delay(1000);
  my_power.set12V( true );
  delay(1000);
  
  Serial.begin(115200);
  Serial1.begin(115200);
  gps.initialize();
  motor.initialize();
  gumstix.initialize();
}

void publishSensorReadings() {
  my_compass.updateHPR();
  int heading = (int) (my_compass.heading*10);
  int rate = (int) (my_gyro.getOmega()*100);
  int voltage = (int) (my_power.getVoltage()*100);
  int temp = (int) (my_temp.getTemp() * 10);
  int humidity = (int) (my_humidity.getHumidity());
  int thrust_current = (int) (motor.readCurrent()*100);
  char output [200];
  sprintf( &output[0], "heading=%d,rate=%d,voltage=%d,temp=%d,humidity=%d,thrust_current=%d", 
    heading, rate, voltage, temp, humidity, thrust_current );
//  Serial.println( output );
  Serial1.println( output );
}

const int publishPeriod = 500;

boolean mains_enabled = true;
unsigned long last_time = 0, last_time2 = 0;
//int something = 0;

void loop() {
  delay(1);
  
  double voltage = my_power.getVoltage();
  if ( voltage < minimumVoltage && mains_enabled ) {
    mains_enabled = false;
    motor.setEnable( false );
    Serial.println("Low voltage - motor shutdown.");
  }
  
  motor.setVoltage( voltage );
  motor.doWork();
  gps.doWork();
  gumstix.doWork();
  
  if ( millis() - gumstix.last_motor_command_time > 1000 ) {
    motor.setPWM(0);
    Serial.println("Motor command timeout.");
  }
  
  if ( gumstix.new_motor_command ) {
    gumstix.new_motor_command = false;
    motor.setPWM(gumstix.motor_thrust);
    motor.setAngle(gumstix.motor_rudder);
    Serial.print("thrust: ");
    Serial.print(gumstix.motor_thrust);
    Serial.print("  rudder: ");
    Serial.println(gumstix.motor_rudder);
  }
  
  if ( gumstix.compass_calibration_signal ) {
     if ( my_compass.inCalibrationMode ) {
       my_compass.stopCalibration();
       Serial.println("Exiting compass calibration.");
       Serial1.println("Exiting compass calibration.");
     } else {
       my_compass.startCalibration();
       Serial.println("Starting compass calibration.");
       Serial1.println("Starting compass calibration.");
     }
     gumstix.compass_calibration_signal = false;
  }
  
  if ( millis() - last_time2 > publishPeriod ) {
    publishSensorReadings();
    last_time2 = millis(); 
  }
  
//  if ( millis() - last_time > 5000 ) {
//    if (something ==0 )
//      motor.setPWM(50);
//    else if ( something == 1)
//      motor.setPWM(80);
//    else {
//      motor.setPWM(0);
//      something = -1;
//    }
//    something++;
//    last_time = millis();
//  }
}
