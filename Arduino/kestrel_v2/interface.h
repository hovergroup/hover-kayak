#ifndef INTERFACE_H
#define INTERFACE_H

#include <Arduino.h>

extern float core_looprate;

#include "sbus.h"
#include "roboteq.h"
#include "radio.h"

extern SBUS sbus;
extern Roboteq roboteq;
extern RadioControl radio;

class Interface
{
private:
  enum State {
    main_menu,
    radio_menu
  };
  
public:
  Interface(Stream& port) : _port(port) {
    last_incoming_time = 0;
  }
  
  void doWork();
  
private:
  Stream& _port;
  
  void printMainMenu();
  void mainMenuSelect(int rc);
  
  void printRoboteqStatus();
  void printRCStatus();
  void printStatus();
  void printRadioStatus();
    
  void printLine(String s, int val);
  void printLine(String s, float val);
  void printLine(String s, unsigned long val);
  void printBool(String s, boolean val, String pos, String neg);
  
  void handleIncoming();
  void handleRadioMenu();
  
  unsigned long last_incoming_time;
  State m_state;
  
};

#endif
