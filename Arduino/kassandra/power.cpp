#include "power.h"

Power::Power() {
  pinMode(mainBattery5vRelayPin, OUTPUT);
  pinMode(mainBattery12vRelayPin, OUTPUT);
  pinMode(auxBatteryRelayPin, OUTPUT);
}

float Power::getVoltage() {  
  int pini = analogRead(voltageDividerPinNumber);
//  Serial.println(pini);
  int pinv = map(pini, 0, 1023, 0, 5000);
  return (float)pinv * gain/1000.0;
}

void Power::set12V( boolean onoff ) {
  if ( onoff )
    digitalWrite(mainBattery12vRelayPin, HIGH);
  else
    digitalWrite(mainBattery12vRelayPin, LOW);
}

void Power::set5V( boolean onoff ) {
  if ( onoff )
    digitalWrite(mainBattery5vRelayPin, HIGH);
  else
    digitalWrite(mainBattery5vRelayPin, LOW);
}

void Power::setAux( boolean onoff ) {
  if ( onoff )
    digitalWrite(auxBatteryRelayPin, LOW);
  else
    digitalWrite(auxBatteryRelayPin, HIGH);
}

