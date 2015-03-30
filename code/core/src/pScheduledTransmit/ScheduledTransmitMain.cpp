/*
 * pScheduledTransmit
 *        File: ScheduledTransmitMain.cpp
 *  Created on: Apr 9, 2013
 *      Author: Josh Leighton
 */

#include <string>
#include "MBUtils.h"
#include "ColorParse.h"
#include "MOOS/libMOOS/MOOSLib.h"
#include "ScheduledTransmit.h"
#include "ScheduledTransmit_Info.h"
#include <boost/lexical_cast.hpp>

using namespace std;

int main(int argc, char *argv[]) {
    string mission_file;
    string run_command = "pScheduledTransmit";

    ScheduledTransmit scheduledTransmit;

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
        else if (strBegins(argi, "--period=")) {
            try {
                double period = boost::lexical_cast<double>(argi.substr(9));
                scheduledTransmit.setPeriod(period);
                cout << "Period set to " << period << endl;
            } catch (const boost::bad_lexical_cast &) {
                cout << "Invalid period." << endl;
                exit(1);
            }
        } else if (strBegins(argi, "--offset=")) {
            try {
                double offset = boost::lexical_cast<double>(argi.substr(9));
                scheduledTransmit.setOffset(offset);
                cout << "Offset set to " << offset << endl;
            } catch (const boost::bad_lexical_cast &) {
                cout << "Invalid offset." << endl;
                exit(1);
            }
        } else if (i == 2)
            run_command = argi;
    }

    if (mission_file == "")
        showHelpAndExit();

    cout << termColor("green");
    cout << "pScheduledTransmit launching as " << run_command << endl;
    cout << termColor() << endl;

    scheduledTransmit.Run(run_command.c_str(), mission_file.c_str());

    return (0);
}

