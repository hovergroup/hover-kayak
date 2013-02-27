
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
#define azimuthCenter 1462
#define azimuth60Range 587

// thrust limiting when turning
#define limitStartAngle 30 // angle past which limiting should start
#define limitOffset 0 // ammount shich should be limited at start angle
#define limitSlope 20 // additional percent limited per degree past start angle

