/*
 * iAcommsDriver
 *        File: acomms_driverMain.cpp
 *  Created on: May 24, 2012
 *      Author: Josh Leighton
 */

#include <string>
#include "MOOS/libMOOS/MOOSLib.h"
#include "acomms_driver.h"

using namespace std;

int main(int argc, char *argv[]) {
    // default parameters file
    string sMissionFile = "acomms_driver.moos";

    //under what name shoud the application register with the MOOSDB?
    string sMOOSName = "iAcommsDriver";

    switch (argc) {
    case 3:
        //command line says don't register with default name
        sMOOSName = argv[2];
    case 2:
        //command line says don't use default config file
        sMissionFile = argv[1];
    }

    //make an application
    acomms_driver acomms_driverApp;

    //run it
    acomms_driverApp.Run(sMOOSName.c_str(), sMissionFile.c_str());

    return (0);
}

