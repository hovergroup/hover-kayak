/*
 * pScheduledTransmit
 *        File: ScheduledTransmit_Info.cpp
 *  Created on: May 19, 2014
 *      Author: Josh Leighton
 */

#include <cstdlib>
#include <iostream>
#include "ScheduledTransmit_Info.h"
#include "ColorParse.h"
#include "ReleaseInfo.h"

using namespace std;

//----------------------------------------------------------------
// Procedure: showSynopsis

void showSynopsis() {
    blk("SYNOPSIS:                                                       ");
    blk("------------------------------------                            ");
    blk("  pScheduledTransmit is used to periodically send random acomms ");
    blk("  data.                                                         ");
    blk("                                                                ");
}

//----------------------------------------------------------------
// Procedure: showHelpAndExit

void showHelpAndExit() {
    blk("                                                                ");
    blu("=============================================================== ");
    blu("Usage: pAcommsSimulator file.moos [OPTIONS]                   ");
    blu("=============================================================== ");
    blk("                                                                ");
    showSynopsis();
    blk("                                                                ");
    blk("Options:                                                        ");
    mag("  --alias", "=<ProcessName>                                      ");
    blk("      Launch pAcommsSimulator with the given process name         ");
    blk("      rather than pAcommsSimulator.                           ");
    mag("  --example, -e                                                 ");
    blk("      Display example MOOS configuration block.                 ");
    mag("  --help, -h                                                    ");
    blk("      Display this help message.                                ");
    mag("  --interface, -i                                               ");
    blk("      Display MOOS publications and subscriptions.              ");
    mag("  --version,-v                                                  ");
    blk("      Display the release version of pAcommsSimulator.          ");
    mag("  --period=[period in seconds]                                  ");
    blk("      Set a custom period, ignoring that set in the MOOS file.  ");
    mag("  --offset=[offset in seconds]                                  ");
    blk("      Set a custom offset, ignoring that set in the MOOS file.  ");
    blk("                                                                ");
    blk("Note: If argv[2] does not otherwise match a known option,       ");
    blk("      then it will be interpreted as a run alias. This is       ");
    blk("      to support pAntler launching conventions.                 ");
    blk("                                                                ");
    exit(0);
}

//----------------------------------------------------------------
// Procedure: showExampleConfigAndExit

void showExampleConfigAndExit() {
    blk("                                                                ");
    blu("=============================================================== ");
    blu("pAcommsSimulator Example MOOS Configuration                   ");
    blu("=============================================================== ");
    blk("                                                                ");
    blk("ProcessConfig = pAcommsSimulator                              ");
    blk("{                                                               ");
    blk("  AppTick   = 4                                                 ");
    blk("  CommsTick = 4                                                 ");
    blk("                                                                ");
    blk("  period = 10                                                   ");
    blk("  offset = 0                                                   ");
    blk("}                                                               ");
    blk("                                                                ");
    exit(0);
}

//----------------------------------------------------------------
// Procedure: showInterfaceAndExit

void showInterfaceAndExit() {
    blk("                                                                ");
    blu("=============================================================== ");
    blu("pAcommsSimulator INTERFACE                                    ");
    blu("=============================================================== ");
    blk("                                                                ");
    showSynopsis();
    blk("                                                                ");
    blk("SUBSCRIPTIONS:                                                  ");
    blk("------------------------------------                            ");
    blk("  SCHEDULED_TRANSMITS=[ON/OFF]                                  ");
    blk("  SCHEDULED_TRANSMITS_PERIOD=[seconds]                          ");
    blk("  SCHEDULED_TRANSMITS_OFFSET=[seconds]                          ");
    blk("                                                                ");
    blk("PUBLICATIONS:                                                   ");
    blk("------------------------------------                            ");
    blk("  ACOMMS_TRANSMIT_DATA filled with random ASCII content.        ");
    blk("                                                                ");
    exit(0);
}

//----------------------------------------------------------------
// Procedure: showReleaseInfoAndExit

void showReleaseInfoAndExit() {
    showReleaseInfo("pAcommsSimulator", "gpl");
    exit(0);
}

