/*
 * iOS5000_Hover
 *        File: main.cpp
 *  Created on: Aug 9, 2012
 *      Author: Josh Leighton
 */

#include <string>
#include "MBUtils.h"
#include "iOS5000.h"

using namespace std;

int main(int argc, char *argv[]) {

    // Look for a request for version information
    if (scanArgs(argc, argv, "-v", "--version", "-version")) {
        //showReleaseInfo("pXRelay", "gpl");
        return (0);
    }

    string sMissionFile = "Mission.moos";
    string sMOOSName = "iOS5000_Hover";

    switch (argc) {
    case 3:
        sMOOSName = argv[2];
    case 2:
        sMissionFile = argv[1];
    }

    cout << sMOOSName << "  " << sMissionFile << endl;

    iOS5000_Hover mine;

    mine.Run(sMOOSName.c_str(), sMissionFile.c_str());

    return (0);
}

