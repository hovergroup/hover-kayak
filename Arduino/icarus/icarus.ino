
static const float Rhigh = 6;
static const float Rlow = 3.14;

void setup() {  
  pinMode(A0, INPUT);
  
  Serial1.begin(9600);
}

void loop() {
  delay(500);
  int val = analogRead(A0);
  float pinvoltage = val / 3.3;
  float batteryvoltage = pinvoltage * (Rhigh + Rlow) / Rlow;
  Serial1.println(batteryvoltage);
}
