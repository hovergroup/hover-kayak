/*
 * iIcarus
 *        File: main.cpp
 *  Created on: Mar 11, 2014
 *      Author: Josh Leighton
 */

#include <string>
#include "MBUtils.h"
#include "icarus.h"

using namespace std;

int main(int argc, char *argv[]) {

    // Look for a request for version information
    if (scanArgs(argc, argv, "-v", "--version", "-version")) {
        //showReleaseInfo("pXRelay", "gpl");
        return (0);
    }

    string sMissionFile = "Mission.moos";
    string sMOOSName = "iIcarus";

    switch (argc) {
    case 3:
        sMOOSName = argv[2];
    case 2:
        sMissionFile = argv[1];
    }

    cout << sMOOSName << "  " << sMissionFile << endl;

    icarus_driver mine;

    mine.Run(sMOOSName.c_str(), sMissionFile.c_str());

    return (0);
}

