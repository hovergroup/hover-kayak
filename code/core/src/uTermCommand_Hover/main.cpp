/*
 * uTermCommand_Hover
 *        File: main.cpp
 *  Created on: May 19, 2014
 *      Author: Josh Leighton
 */

/*****************************************************************/
/*    NAME: Michael Benjamin, Henrik Schmidt, and John Leonard   */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: main.cpp                                             */
/*    DATE: June 26th 2007                                       */
/*                                                               */
/* This program is free software; you can redistribute it and/or */
/* modify it under the terms of the GNU General Public License   */
/* as published by the Free Software Foundation; either version  */
/* 2 of the License, or (at your option) any later version.      */
/*                                                               */
/* This program is distributed in the hope that it will be       */
/* useful, but WITHOUT ANY WARRANTY; without even the implied    */
/* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR       */
/* PURPOSE. See the GNU General Public License for more details. */
/*                                                               */
/* You should have received a copy of the GNU General Public     */
/* License along with this program; if not, write to the Free    */
/* Software Foundation, Inc., 59 Temple Place - Suite 330,       */
/* Boston, MA 02111-1307, USA.                                   */
/*****************************************************************/

#include <cstring>
#include "TermUtils.h"
#include "MBUtils.h"
#include "TermCommand.h"
#include "ColorParse.h"
#include "TermCommand_Info.h"
#include "MOOSAppRunnerThread.h"

using namespace std;

//--------------------------------------------------------
// Procedure: main

int main(int argc, char * argv[]) {
    string mission_file;
    string run_command = argv[0];
    string verbose_setting;

    for (int i = 1; i < argc; i++) {
        string argi = argv[i];
        if ((argi == "-v") || (argi == "--version") || (argi == "-version"))
            showReleaseInfoAndExit();
        else if ((argi == "-e") || (argi == "--example")
                || (argi == "-example"))
            showExampleConfigAndExit();
        else if ((argi == "-h") || (argi == "--help") || (argi == "-help"))
            showHelpAndExit();
        else if ((argi == "-i") || (argi == "--interface"))
            showInterfaceAndExit();
        else if (strEnds(argi, ".moos") || strEnds(argi, ".moos++"))
            mission_file = argv[i];
        else if (strBegins(argi, "--alias="))
            run_command = argi.substr(8);
        else if (strBegins(argi, "--verbose="))
            verbose_setting = tolower(argi.substr(10));
        else if (i == 2)
            run_command = argi;
    }

    if (mission_file == "")
        showHelpAndExit();

    cout << termColor("green");
    cout << "uTermCommand launching as " << run_command << endl;
    cout << termColor() << endl;

    TermCommand theTermCommand;
    MOOSAppRunnerThread appThread(&theTermCommand, run_command.c_str(),
            mission_file.c_str());

    bool quit = false;
    while (!quit) {
        char c = getCharNoWait();
        if ((c == 'q') || (c == (char) (3))) {  // ASCII 03 is control-c
            quit = true;
            cout << "Quitting....." << endl;
        } else {
            theTermCommand.m_tc_mutex.Lock();
            theTermCommand.handleCharInput(c);
            theTermCommand.m_tc_mutex.UnLock();
        }
    }

    appThread.quit();
    return (0);
}

