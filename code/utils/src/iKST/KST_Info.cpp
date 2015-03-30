/*
 * iKST
 *        File: KST_Info.cpp
 *  Created on: Aug 12, 2013
 *      Author: Josh Leighton
 */

#include <cstdlib>
#include <iostream>
#include "KST_Info.h"
#include "ColorParse.h"
#include "ReleaseInfo.h"

using namespace std;

//----------------------------------------------------------------
// Procedure: showSynopsis

void showSynopsis() {
    blk("SYNOPSIS:                                                       ");
    blk("------------------------------------                            ");
    blk("  The iKST application is used for creating a comma delimited   ");
    blk("  file for reading by the plotting utility kst. Variables can be");
    blk("  specified in either the process config block or at the command");
    blk("  line. Output occurs at the rate of AppTick (default 4).       ");
    blk("                                                                ");
}

//----------------------------------------------------------------
// Procedure: showHelpAndExit

void showHelpAndExit() {
    blk("                                                                ");
    blu("=============================================================== ");
    blu("Usage: iKST file.moos [OPTIONS]                   ");
    blu("=============================================================== ");
    blk("                                                                ");
    showSynopsis();
    blk("                                                                ");
    blk("Options:                                                        ");
    mag("  --alias", "=<ProcessName>                                     ");
    blk("      Launch iKST with the given process name                   ");
    blk("      rather than iKST.                                         ");
    mag("  --example, -e                                                 ");
    blk("      Display example MOOS configuration block.                 ");
    mag("  --help, -h                                                    ");
    blk("      Display this help message.                                ");
    mag("  --interface, -i                                               ");
    blk("      Display MOOS publications and subscriptions.              ");
    mag("  --version,-v                                                  ");
    blk("      Display the release version of iKST.                      ");
    mag("  --vars=[comma delimited variable names]                       ");
    blk("      Specify variables to log (in addition to those specified  ");
    blk("      in MOOS file).                                            ");
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
    blu("iKST Example MOOS Configuration                   ");
    blu("=============================================================== ");
    blk("                                                                ");
    blk("ProcessConfig = iKST                              ");
    blk("{                                                               ");
    blk("  IterateMode = 0    // regular iterate and mail                ");
    blk("  AppTick     = 4                                               ");
    blk("                                                                ");
    blk("  output_path = \"/home/josh/kst_log.csv\"                      ");
    blk("                                                                ");
    blk("  LOG = NAV_X                                                   ");
    blk("  LOG = NAV_Y                                                   ");
    blk("}                                                               ");
    blk("                                                                ");
    exit(0);
}

//----------------------------------------------------------------
// Procedure: showInterfaceAndExit

void showInterfaceAndExit() {
    blk("                                                                ");
    blu("=============================================================== ");
    blu("iKST INTERFACE                                    ");
    blu("=============================================================== ");
    blk("                                                                ");
    showSynopsis();
    blk("                                                                ");
    blk("SUBSCRIPTIONS:                                                  ");
    blk("------------------------------------                            ");
    blk("  Determined by variables requested for logging.                ");
    blk("                                                                ");
    blk("PUBLICATIONS:                                                   ");
    blk("------------------------------------                            ");
    blk("  None                                                          ");
    blk("                                                                ");
    exit(0);
}

//----------------------------------------------------------------
// Procedure: showReleaseInfoAndExit

void showReleaseInfoAndExit() {
    showReleaseInfo("iKST", "gpl");
    exit(0);
}

