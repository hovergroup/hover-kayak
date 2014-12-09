/*
 * iKST
 *        File: main.cpp
 *  Created on: Aug 12, 2013
 *      Author: Josh Leighton
 */

#include <string>
#include "MBUtils.h"
#include "ColorParse.h"
#include "KST.h"
#include "KST_Info.h"

using namespace std;

int main(int argc, char *argv[]) {
    string mission_file;
    string run_command = "iKST";

    KST kst;

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
        else if (strBegins(argi, "--vars=")) {
            string right = argi.substr(7);
            string var = MOOSChomp(right, ",");
            while (!right.empty()) {
                kst.m_vars.push_back(var);
                var = MOOSChomp(right, ",");
            }
            kst.m_vars.push_back(var);
            cout << "Reservations specified at command line: " << endl;
            for (int i=0; i<kst.m_vars.size(); i++) {
                cout << "    " << kst.m_vars[i] << endl;
            }
        } else if (i == 2)
            run_command = argi;
    }

    if (mission_file == "")
        showHelpAndExit();

    cout << termColor("green");
    cout << "iKST launching as " << run_command << endl;
    cout << termColor() << endl;

    kst.Run(run_command.c_str(), mission_file.c_str());

    return (0);
}

