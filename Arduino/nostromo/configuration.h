

#define SBUS_SIGNAL_OK          0x00
#define SBUS_SIGNAL_LOST        0x01
#define SBUS_SIGNAL_FAILSAFE    0x03

#define mainBattery5vRelayPin 56
#define mainBattery12vRelayPin 55
#define servoPowerRelayPin 57

#define voltageDividerGain 23
#define voltageDividerPinNumber 4

#define gyroAnalogPin 8
#define temperatureAnalogPin 7
#define humidityAnalogPin 6

#define compassOffset 0

#define azimuthServoPinNumber A0
#define azimuthCenter 1462
#define azimuth60Range 587
//#define azimuthLeft60 875
//#define azimuthRight60 2050

#define thrusterAddress 128
#define thrusterEStopPin 7

#define thrusterSlewLimit 100 // percent per second
#define degreesPerMicrosecond 0.15

#define minimumVoltage 12.4
