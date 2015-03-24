/*
 * pProtoReportParser
 *        File: ProtoReportParser.h
 *  Created on: Sep 11, 2013
 *      Author: Josh Leighton
 */

#ifndef ProtoReportParser_HEADER
#define ProtoReportParser_HEADER

#include "MOOS/libMOOS/MOOSLib.h"
#include "reports.pb.h"
#include "NodeRecord.h"
#include <map>

class ProtoReportParser: public CMOOSApp {
public:
    ProtoReportParser();
    ~ProtoReportParser();

protected:
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();
    void RegisterVariables();

private:
    // Configuration variables

private:
    std::map<std::string, bool> active_secondaries;

    // State variables
};

#endif 
