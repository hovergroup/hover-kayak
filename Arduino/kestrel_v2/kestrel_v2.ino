// pin configuration
#define RelayPinVbatt 56
#define RelayPin12V 55

// thrust limiting when turning
#define limitStartAngle 30 // angle past which limiting should start
#define limitOffset 0 // ammount shich should be limited at start angle
#define limitSlope 10 // additional thrust limited per degree past start angle

#include <Wire.h>
#include <Servo.h>
#include "interface.h"
#include "roboteq.h"
#include "sbus.h"
#include "radio.h"
#include "azimuth.h"
#include "tmp102.h"
#include "battery.h"
#include "gumstix.h"

SBUS sbus = SBUS(Serial3);
Interface interface = Interface(Serial);
Roboteq roboteq = Roboteq(Serial2);
RadioControl radio = RadioControl();
Azimuth azimuth = Azimuth();
TMP102 tmp102 = TMP102();
Battery battery = Battery();
GumstixSerial gumstix = GumstixSerial(Serial1);

void setup() {
  Wire.begin();
  
  pinMode(RelayPinVbatt, OUTPUT);
  digitalWrite(RelayPinVbatt, HIGH);
  pinMode(RelayPin12V, OUTPUT);
  digitalWrite(RelayPin12V, HIGH);
  
  radio.initialize();
  azimuth.initialize();
  radio.powerBullet();
    
  Serial.begin(115200);
  Serial.setTimeout(1);
  
  Serial3.begin(100000);
  Serial1.begin(115200);
  Serial2.begin(115200);
  delay(500);
}

// timing
int iteration = 0;
float core_looprate;
unsigned long command_timer_start = 0;

// control
int desired_thrust, desired_rudder;
int thrust_limit = 1000;


void loop() {
  sbus.doWork();
  interface.doWork();
  roboteq.doWork();
  battery.doWork();
  gumstix.doWork();
  
  // inform azimuth of e-stop and thrust limit settings
  azimuth.setEStop(roboteq.getStopSwitch());
  azimuth.setThrustLimit(battery.getThrustLimit());
  
  // go through potential command sources
  if (interface.getManualEnabled()) { // get commands from manual input
    desired_thrust = interface.getThrustCommand();
    desired_rudder = interface.getRudderCommand();
    interface.setCommandSource(s_manual);
  } else if (sbus.getRcAvailable() && !sbus.getSerialEnable()) { // get commands from rc
    interface.setCommandSource(s_rc);
    desired_rudder = sbus.getRudder();
    if (sbus.getThrustEnable())
      desired_thrust = sbus.getThrust();
    else
      desired_thrust = 0;
  } else if (gumstix.getCommandsAvailable()) { // get commands from gumstix
    desired_thrust = gumstix.getThrustCommand();
    desired_rudder = gumstix.getRudderCommand();
    interface.setCommandSource(s_gumstix);
  } else { // do nothing
    desired_thrust = 0;
    desired_rudder = 0;
    interface.setCommandSource(s_no_input);
  }
  
  // send rudder command
  azimuth.setAngle(desired_rudder);
  
  // apply battery thrust limiting
  thrust_limit = battery.getThrustLimit();
  desired_thrust = constrain(desired_thrust, -thrust_limit, thrust_limit);
  
  // apply turn thrust limiting
  int current_angle = azimuth.getCurrentAngle();
  if ( abs(current_angle) >= limitStartAngle ) {
    int turn_limit = (abs(current_angle)-limitStartAngle)*limitSlope + limitOffset;
    desired_thrust = constrain( desired_thrust, -1000+turn_limit, 1000-turn_limit );
  }
  
  // send thrust command
  roboteq.setPower(desired_thrust);
  
  // timer
  float command_timer_elapsed = millis() - command_timer_start;
  if (command_timer_elapsed > 5000) {
    roboteq.rolloverCounters(command_timer_elapsed/1000.0);
    gumstix.rolloverCounters(command_timer_elapsed/1000.0);
    core_looprate = iteration*1000.0/command_timer_elapsed;
    tmp102.updateReading();
    command_timer_start = millis();
    iteration=0;
  }
  iteration++;
  delay(1);
}
