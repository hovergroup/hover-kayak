#include "Arduino.h"
#include "Wire.h"

#include "tmp102.h"

static const float divider = 5.54;
static const float fudge = 0.1;

TMP102 tmp102 = TMP102();

void setup() {  
  pinMode(A0, INPUT);
  
  Wire.begin();
  
  Serial.begin(9600);
}

void loop() {
  delay(500);
  int val = analogRead(A0);
  float pinvoltage = val * 5.0 / 1024;
  float batteryvoltage = pinvoltage * divider + 0.1;
  tmp102.updateReading();
  Serial.print(tmp102.getTemp());
  Serial.print("   ");
  Serial.println(batteryvoltage);
}
