/*
 * iAltimeter_cruzPro
 *        File: iAltimeter_cruzPro_nmea.cpp
 *  Created on: Sept 27, 2013
 *      Author: Josh Leighton
 */

#include "Altimeter_cruzPro.h"
#include "MBUtils.h"

using namespace std;
using namespace boost::posix_time;

void Altimeter_cruzPro::parseSDDPT(string sNMEAString) {
    string sTmp;

    // field 1 - depth
    sTmp = MOOSChomp(sNMEAString, ",");
    if (sTmp.size() == 0)
        return;
    double depth = atof(sTmp.c_str());
    m_Comms.Notify("ALTIMETER_DEPTH", depth);
}

void Altimeter_cruzPro::parseLine(string msg) {
    writeLine(msg);

    if (!DoNMEACheckSum(msg)) {
        cout << "checksum failed on: " << msg << endl;
        for (int i = 0; i < msg.size(); i++) {
            cout << hex << (int) msg[i] << " ";
        }
        cout << endl;
        return;
    }

    string cmd = MOOSChomp(msg, ",");
//  cout << "command = " << cmd << endl;
    if (cmd == "$SDDPT")
        parseSDDPT(msg);

}
