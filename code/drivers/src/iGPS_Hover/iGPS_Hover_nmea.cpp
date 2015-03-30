/*
 * iGPS_Hover
 *        File: iGPS_Hover_nmea.cpp
 *  Created on: Jul 24, 2012
 *      Author: Josh Leighton
 */

#include "simple_gps.h"
#include "MBUtils.h"

using namespace std;
using namespace boost::posix_time;

void SIMPLE_GPS::parseGPRMC(string sNMEAString) {
//	cout << "rmc: " << sNMEAString << endl;
    bool bGood = true;

    string sTmp;

    // field 1 - UTC time
    sTmp = MOOSChomp(sNMEAString, ",");
    if (sTmp.size() == 0)
        return;
    string sTime = sTmp;
//	double dfTime = atof( sTmp.c_str() );

// field 2 - fix status
    sTmp = MOOSChomp(sNMEAString, ",");
    if (sTmp == "A")
        m_Comms.Notify("GPS_LOCK", 1.0);
    else
        m_Comms.Notify("GPS_LOCK", 0.0);

    // field 3 - latitude
    sTmp = MOOSChomp(sNMEAString, ",");

    // field 4 - N/S
    sTmp = MOOSChomp(sNMEAString, ",");

    // field 5 - longitude
    sTmp = MOOSChomp(sNMEAString, ",");

    // field 6 - E/W
    sTmp = MOOSChomp(sNMEAString, ",");

    // field 7 - speed
    sTmp = MOOSChomp(sNMEAString, ",");
    double dfSpeed = atof(sTmp.c_str())*.5144;
    m_Comms.Notify("GPS_SPEED", dfSpeed);

    // field 8 - course
    sTmp = MOOSChomp(sNMEAString, ",");
    double dfCourse = atof(sTmp.c_str());
    m_Comms.Notify("GPS_HEADING", dfCourse);

    // field 9 - date
    sTmp = MOOSChomp(sNMEAString, ",");
    if (sTmp.size() != 6)
        return;
//	double dfDate = atof( sTmp.c_str() );
    string sDate = sTmp;

    // confirm posix date construction
    string modified_date = "20" + sDate.substr(4, 2) + sDate.substr(2, 2)
            + sDate.substr(0, 2);
    string composite = modified_date + "T" + sTime;
    try {
        ptime t(from_iso_string(composite));
        m_Comms.Notify("GPS_PTIME", to_simple_string(t));
        double seconds = t.time_of_day().total_milliseconds() / 1000.0;
        m_Comms.Notify("GPS_TIME_SECONDS", seconds);
    } catch (...) {
        cout << "Exception constructing ptime" << endl;
    }
}

void SIMPLE_GPS::parseGPGST(string sNMEAString) {
    string sTmp;

    // field 1 - UTC time
    sTmp = MOOSChomp(sNMEAString, ",");

    // field 2 - rms of std. devs
    sTmp = MOOSChomp(sNMEAString, ",");
    if (sTmp.size() == 0)
        return;
    double range_rms = atof(sTmp.c_str());
    m_Comms.Notify("GPS_RANGE_RMS", range_rms);

    // field 3 - not supported
    sTmp = MOOSChomp(sNMEAString, ",");

    // field 4 - not supported
    sTmp = MOOSChomp(sNMEAString, ",");

    // field 5 - not supported
    sTmp = MOOSChomp(sNMEAString, ",");

    // field 6 - std dev of latitude
    sTmp = MOOSChomp(sNMEAString, ",");
    if (sTmp.size() == 0)
        return;
    double std_lat = atof(sTmp.c_str());

    // filed 7 - std dev of longitude
    sTmp = MOOSChomp(sNMEAString, ",");
    if (sTmp.size() == 0)
        return;
    double std_long = atof(sTmp.c_str());

    // field 8 - std dev of altitude
    sTmp = MOOSChomp(sNMEAString, ",");
    if (sTmp.size() == 0)
        return;
    double std_alt = atof(sTmp.c_str());
}

void SIMPLE_GPS::parseGPGGA(string sNMEAString) {
//	cout << "gga: " << sNMEAString << endl;
    bool bGood = true;

    string sTmp;

    // field 1 - UTC time
    sTmp = MOOSChomp(sNMEAString, ",");

    // field 2 - latitude
    sTmp = MOOSChomp(sNMEAString, ",");
    if (sTmp.size() == 0)
        return;
    double dfLat = atof(sTmp.c_str());

    // field 3 - N/S
    sTmp = MOOSChomp(sNMEAString, ",");
    if (sTmp == "S")
        dfLat *= -1.0;

    // field 4 - longitude
    sTmp = MOOSChomp(sNMEAString, ",");
    if (sTmp.size() == 0)
        return;
    double dfLong = atof(sTmp.c_str());

    // field 5 - E/W
    sTmp = MOOSChomp(sNMEAString, ",");
    if (sTmp == "W")
        dfLong *= -1.0;

    // perform lat/lon conversion
    double dfLatDecDeg = m_Geodesy.DMS2DecDeg(dfLat);
    double dfLongDecDeg = m_Geodesy.DMS2DecDeg(dfLong);
    m_Comms.Notify("GPS_LATITUDE", dfLatDecDeg);
    m_Comms.Notify("GPS_LONGITUDE", dfLongDecDeg);
    double dfXLocal, dfYLocal;
    if (m_Geodesy.LatLong2LocalUTM(dfLatDecDeg, dfLongDecDeg, dfYLocal,
            dfXLocal)) {
        m_Comms.Notify("GPS_X", dfXLocal);
        m_Comms.Notify("GPS_Y", dfYLocal);
    }

    // field 6 - position fix status
    sTmp = MOOSChomp(sNMEAString, ",");
    int dfFix = atoi(sTmp.c_str());
    switch (dfFix) {
    case 0:
        m_Comms.Notify("GPS_FIX_STATUS", "invalid");
        break;
    case 1:
        m_Comms.Notify("GPS_FIX_STATUS", "standard");
        break;
    case 2:
        m_Comms.Notify("GPS_FIX_STATUS", "differential");
        break;
    }

    // field 7 - num satellites
    sTmp = MOOSChomp(sNMEAString, ",");
    int dfNumSV = atoi(sTmp.c_str());
    m_Comms.Notify("GPS_NUM_SV", dfNumSV);

    // field 8 - HDOP
    sTmp = MOOSChomp(sNMEAString, ",");
    double dfHDOP = atof(sTmp.c_str());
    m_Comms.Notify("GPS_HDOP", dfHDOP);
}

void SIMPLE_GPS::parseLine(string msg) {
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
//	cout << "command = " << cmd << endl;
    if (cmd == "$GPRMC")
        parseGPRMC(msg);
    else if (cmd == "$GPGGA")
        parseGPGGA(msg);
    else if (cmd == "$GPGST")
        parseGPGST(msg);

}
