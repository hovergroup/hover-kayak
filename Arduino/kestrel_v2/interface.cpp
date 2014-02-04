#include "interface.h"

#define restartPin 12

char RESTART_STRING[] = "restart";

void Interface::printMainMenu() {
  _port.println("Main Menu");
  _port.println("1        Status");
  _port.println("2        RC");
  _port.println("3        Roboteq");
  _port.println("4        Radio");
  _port.println("5        Manual Control");
  _port.println("6        Power");
  _port.println("7        Gumstix");
  _port.println("9        Restart");
  _port.println();
}

void Interface::doWork() {
  // watch timeout in manual control
  if (m_state == manual_control && millis() - last_manual_command > 30000) {
    manual_enabled = false;
    desired_thrust = 0;
    desired_rudder = 0;
    m_state = main_menu;
    printMainMenu(); 
  }
  
  if (_port.available() != 0) {
    handleIncoming();
  }
}

void Interface::handleIncoming() {
  // timeout and return to main menu if more than 30sec elapsed
  if (millis() - last_incoming_time > 30000 || last_incoming_time == 0) {
    _port.println("Timeout - last input ignored.");
    _port.println();
    
    m_state = main_menu;
    printMainMenu(); 
    
    // flush receive buffer
    while (_port.available() != 0) {
      _port.read();
    }
    last_incoming_time = millis();
    return;
  }
  
  last_incoming_time = millis();
   
  int rc;
  switch (m_state) {
    case main_menu:
      rc = _port.parseInt();
      if (rc == 0) break;
      else mainMenuSelect(rc);
      break;
      
    case radio_menu:
      handleRadioMenu();
      break;
      
    case manual_control:
      handleManualControl();
      break;
      
    case restart_prompt:
      handleRestartPrompt();
      break;
      
    default:
      break;
  }
  while (_port.available() != 0) {
    _port.read();
  }
}

void Interface::handleRestartPrompt() {
  while (_port.available()) {
    char c = _port.read();
    if (c == RESTART_STRING[restart_state]) {
      restart_state++;
    } else {
      m_state = main_menu;
      printMainMenu();
    }
    
    if (restart_state == 7) {
      m_state = main_menu;
      _port.println("\n\nBe right back.");
      pinMode(restartPin, OUTPUT);
      digitalWrite(restartPin, HIGH);
      break;
    }
  }
}

void Interface::handleRadioMenu() {
  int rc = _port.parseInt();
  switch (rc) {
    case 1:
      radio.powerBullet();
      break;
    
    case 2:
      radio.powerFreewave();
      break;
      
    default:
      break;
  }
  printMainMenu();
  m_state = main_menu;
}
  

void Interface::mainMenuSelect(int rc) {
  switch (rc) {
    case 1:
      printStatus();
      break;
      
    case 2:
      printRCStatus();
      break;
      
    case 3:
      printRoboteqStatus();
      break;
      
    case 4:
      printRadioStatus();
      m_state = radio_menu;
      break;
      
    case 5:
      printManualInstructions();
      m_state = manual_control;
      desired_thrust = 0;
      desired_rudder = 0;
      manual_enabled = true;
      last_manual_command = millis();
      break;
      
    case 6:
      printPowerStatus();
      break;
      
    case 7:
      printGumstixStatus();
      break;
      
    case 9:
      printRestartPrompt();
      m_state = restart_prompt;
      restart_state = 0;
      break;
     
    default:
      break;
  } 
}

void Interface::handleManualControl() {
  char c = _port.read();
  while (_port.available()) {
    _port.read();
  }
  
  switch (c) {
    case 'w':
      desired_thrust += 20;
      updateManualControls();
      break;
    case 's':
      desired_thrust -= 20;
      updateManualControls();
      break;
    case 'x':
      desired_thrust = 0;
      updateManualControls();
      break;
    case 'n':
      desired_rudder -= 2;
      updateManualControls();
      break;
    case 'm':
      desired_rudder += 2;
      updateManualControls();
      break;
    case 'b':
      desired_rudder = 0;
      updateManualControls();
      break;
    case ' ':
      updateManualControls();
      break;
    case 'q':
    
      printMainMenu();
      m_state = main_menu;
      manual_enabled = false;
      desired_thrust = 0;
      desired_rudder = 0;
      break;
    default:
      printManualInstructions();
      break;
  }
}

void Interface::updateManualControls() {
  last_manual_command = millis();
  desired_rudder = constrain(desired_rudder, -60, 60);
  desired_thrust = constrain(desired_thrust, -1000, 1000);
  char message[100];
  sprintf( &message[0], "Thrust: %d   Rudder: %d", desired_thrust, desired_rudder);
  _port.println(message);
}

void Interface::printManualInstructions() {
   _port.println("w/s to increase/decrease thrust, x to reset");
   _port.println("n/m to control rudder, b to reset");
   _port.println("press q to return to main menu");
   _port.println("space to refresh command");
   _port.println("control will timeout after 30 seconds of inactivity");
}

void Interface::printPowerStatus() {
   printLine("Voltage: ", roboteq.getBatteryVoltage());
   printLine("Thrust Limit: ", battery.getThrustLimit());
  _port.println();
  printMainMenu();
}

void Interface::printRestartPrompt() {
  _port.println("Are you sure?  Type \"restart\" to confirm.");
}

void Interface::printRadioStatus() {
  printBool("Current radio: ", radio.getPowerIsBullet(), "BULLET", "FREEWAVE");
  _port.println("Press 1 to set power to bullet.");
  _port.println("Press 2 to set power to freewave.");
  _port.println("Press any other key to return to main menu."); 
  _port.println();
}

void Interface::printRoboteqStatus() {
  _port.println("Current Roboteq Status:");
  printBool("EStop: ", roboteq.getStopSwitch(), "ON", "OFF");
  printLine("Voltage: ", roboteq.getBatteryVoltage());
  printLine("Battery Amps: ", roboteq.getBatteryAmps());
  printLine("Motor Amps: ", roboteq.getMotorAmps());
  printLine("Power Output [-1000,1000]: ", roboteq.getPowerOutput());
  printLine("Heatsink Temp (C): ", roboteq.getHeatsinkTemp());
  printLine("Internal Temp (C): ", roboteq.getInternalTemp());
  printLine("Speed Command Rate (Hz): ", roboteq.getSpeedRate());
  printLine("Voltage Read Rate (Hz): ", roboteq.getVoltageRate());
  printLine("Temperature Read Rate (Hz): ", roboteq.getTmpRate());
  printLine("Motor Current Read Rate (Hz): ", roboteq.getMotorAmpRate());
  printLine("Battery Current Read Rate (Hz): ", roboteq.getBatteryAmpRate());
  printLine("EStop Read Rate (Hz): ", roboteq.getStopSwitchRate());
  printLine("Power Read Rate (Hz): ", roboteq.getPowerOutRate());
  printLine("Bad Parse Rate (Hz): ", roboteq.getBadParseRate());
  printLine("Total Bad Parses: ", roboteq.getTotalBadParses());
  _port.println();
  printMainMenu();
}

void Interface::printGumstixStatus() {
  _port.println("Current Gumstix Status:");
  printBool("Commands: ", gumstix.getCommandsAvailable(), "AVAILABLE", "UNAVAILABLE");
  printLine("Motor Command Rate (Hz): ", gumstix.getMotorCommandRate());
  printLine("Bad Parse Rate (Hz): ", gumstix.getBadParseRate());
  printLine("Total Bad Parses: ", gumstix.getTotalBadParseCount());
  _port.println();
  printMainMenu();
}

void Interface::printRCStatus() {
  _port.println("Current RC Status:");
  printBool("RC signal: ", sbus.getRcAvailable(), "AVAILABLE", "UNAVAILABLE");
  printBool("Serial enable: ", sbus.getSerialEnable(), "TRUE", "FALSE");
  printBool("Thrust enable: ", sbus.getThrustEnable(), "TRUE", "FALSE");
  printLine("Thrust: ", sbus.getThrust());
  printLine("Rudder: ", sbus.getRudder());
  _port.println();
  printMainMenu();
}

void Interface::printStatus() {
  printLine("Average looprate (Hz): ", core_looprate);
  _port.println();
  printMainMenu();
}

void Interface::printLine(String s, int val) {
  _port.print(s);
  _port.println(val);
}

void Interface::printLine(String s, float val) {
  _port.print(s);
  _port.println(val);
}


void Interface::printLine(String s, unsigned long val) {
  _port.print(s);
  _port.println(val);
}

void Interface::printBool(String s, boolean val, String pos, String neg) {
  _port.print(s);
  if (val) _port.println(pos);
  else _port.println(neg);
}
