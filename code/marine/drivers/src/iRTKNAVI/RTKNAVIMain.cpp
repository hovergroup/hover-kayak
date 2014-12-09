/*
 * iRTKNAVI
 *        File: RTKNAVIMain.cpp
 *  Created on: May 31, 2013
 *      Author: Josh Leighton
 */

#include <string>
#include "MOOS/libMOOS/MOOSLib.h"
#include "RTKNAVI.h"

using namespace std;

int main(int argc, char *argv[]) {
    // default parameters file
    string sMissionFile = "RTKNAVI.moos";

    //under what name shoud the application register with the MOOSDB?
    string sMOOSName = "iRTKNAVI";

    switch (argc) {
    case 3:
        //command line says don't register with default name
        sMOOSName = argv[2];
    case 2:
        //command line says don't use default config file
        sMissionFile = argv[1];
    }

    //make an application
    RTKNAVI RTKNAVIApp;

    //run it
    RTKNAVIApp.Run(sMOOSName.c_str(), sMissionFile.c_str());

    return (0);
}

