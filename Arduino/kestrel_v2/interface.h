#ifndef INTERFACE_H
#define INTERFACE_H

#include <Arduino.h>

extern float core_looprate;

#include "sbus.h"
#include "roboteq.h"
#include "radio.h"
#include "battery.h"

extern SBUS sbus;
extern Roboteq roboteq;
extern RadioControl radio;
extern Battery battery;

class Interface
{
private:
  enum State {
    main_menu,
    radio_menu,
    manual_control,
    restart_prompt
  };
  
public:
  Interface(Stream& port) : _port(port) {
    last_incoming_time = 0;
    desired_thrust = 0;
    desired_rudder = 0;
    manual_enabled = false;
  }
  
  void doWork();
  
  boolean getManualEnabled() { return manual_enabled; }
  int getThrustCommand() { return desired_thrust; }
  int getRudderCommand() { return desired_rudder; }
  
private:
  Stream& _port;
  
  void printMainMenu();
  void mainMenuSelect(int rc);
  
  void printRoboteqStatus();
  void printRCStatus();
  void printStatus();
  void printRadioStatus();
  void printPowerStatus();
  void printManualInstructions();
  void printRestartPrompt();
  
  void updateManualControls();
    
  void printLine(String s, int val);
  void printLine(String s, float val);
  void printLine(String s, unsigned long val);
  void printBool(String s, boolean val, String pos, String neg);
  
  void handleIncoming();
  void handleRadioMenu();
  void handleManualControl();
  void handleRestartPrompt();
  
  unsigned long last_incoming_time;
  State m_state;
  int restart_state;
  
  boolean manual_enabled;
  int desired_thrust, desired_rudder;
  unsigned long last_manual_command;
  
};

#endif
