/*
 * iAltimeter_cruzPro
 *        File: main.cpp
 *  Created on: Sept 27, 2013
 *      Author: Josh Leighton
 */

#include <string>
#include "MBUtils.h"
#include "ColorParse.h"
#include "Altimeter_cruzPro.h"

using namespace std;

int main(int argc, char *argv[]) {

    // Look for a request for version information
    if (scanArgs(argc, argv, "-v", "--version", "-version")) {
        //showReleaseInfo("pXRelay", "gpl");
        return (0);
    }

    string sMissionFile = "Mission.moos";
    string sMOOSName = "iAltimeter_cruzPro";

    switch (argc) {
    case 3:
        sMOOSName = argv[2];
    case 2:
        sMissionFile = argv[1];
    }

    cout << sMOOSName << "  " << sMissionFile << endl;

    Altimeter_cruzPro Altimeter_cruzPro;

    Altimeter_cruzPro.Run(sMOOSName.c_str(), sMissionFile.c_str());

    return (0);
}

