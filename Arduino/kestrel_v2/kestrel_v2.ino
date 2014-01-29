
// 0-1023 (analogRead) -> v_batt
#define voltageDividerGain 23

// pin configuration
#define mainBattery5vRelayPin 56
#define mainBattery12vRelayPin 55
#define servoPowerRelayPin 57
#define voltageDividerPinNumber 4
#define azimuthServoPinNumber A0
#define thrusterEStopPin 7

// these two values align the servo
#define azimuthCenter 1620
#define azimuth60Range 587

// thrust limiting when turning
#define limitStartAngle 30 // angle past which limiting should start
#define limitOffset 0 // ammount shich should be limited at start angle
#define limitSlope 20 // additional percent limited per degree past start angle

#include "interface.h"
#include "roboteq.h"
#include "sbus.h"
#include "radio.h"

SBUS sbus = SBUS(Serial3);
Interface interface = Interface(Serial);
Roboteq roboteq = Roboteq( Serial2 );
RadioControl radio = RadioControl();

void setup() {
  pinMode(mainBattery5vRelayPin, OUTPUT);
  digitalWrite(mainBattery5vRelayPin, HIGH);
  pinMode(mainBattery12vRelayPin, OUTPUT);
  digitalWrite(mainBattery12vRelayPin, HIGH);
  pinMode(servoPowerRelayPin, OUTPUT);
  pinMode(radioSwitchPin, OUTPUT);
  
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

void loop() {
  sbus.doWork();
  interface.doWork();
  roboteq.doWork();
  
  // timer
  float command_timer_elapsed = millis() - command_timer_start;
  if (command_timer_elapsed > 5000) {
    roboteq.rolloverCounters(command_timer_elapsed/1000.0);
    core_looprate = iteration*1000.0/command_timer_elapsed;
    command_timer_start = millis();
    iteration=0;
  }
  iteration++;
  delay(1);
}
