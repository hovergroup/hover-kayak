/*
 * pAcommsSimulator
 *        File: AcommsSimulator.cpp
 *  Created on: Jan 13, 2014
 *      Author: Josh Leighton
 */

#include <iterator>
#include "MBUtils.h"
#include "AcommsSimulator.h"

using namespace std;

//---------------------------------------------------------
// Constructor

AcommsSimulator::AcommsSimulator() {
    // state variables
    m_channelState = AVAILABLE;

}

//---------------------------------------------------------
// Destructor

AcommsSimulator::~AcommsSimulator() {
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool AcommsSimulator::OnNewMail(MOOSMSG_LIST &NewMail) {
    MOOSMSG_LIST::iterator p;

    for (p = NewMail.begin(); p != NewMail.end(); p++) {
        CMOOSMsg &msg = *p;
        std::string key = msg.GetString();

        // report updates
        if (key == "ACOMMS_SIM_REPORT") {
            AcommsSimReport asr;
            if (asr.ParseFromString(msg.GetString()))
                handleReport(asr);
            else
                cout << "Error parsing report protobuf" << endl;
        }

        // new transmission
        else if (key == "ACOMMS_SIM_IN") {
            goby::acomms::protobuf::ModemTransmission trans;
            if (trans.ParseFromString(msg.GetString())) {
                // get source vehicle name from app name
                std::string app_name = msg.GetSource();
                std::string source_vehicle = MOOSChomp(app_name, "_");

                // check that we have reports from source vehicle
                map<string, AcommsSimReport>::iterator it;
                it = m_vehicleStatus.find(source_vehicle);
                if (it == m_vehicleStatus.end()) {
                    cout << "New transmission error - could not find "
                            << source_vehicle << " among reporting vehicles."
                            << endl;
                } else {
                    handleNewTransmission(trans, source_vehicle);
                }
            } else
                cout << "Error parsing transmission protobuf" << endl;

        }
    }

    return (true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool AcommsSimulator::OnConnectToServer() {
    // register for variables here
    // possibly look at the mission file?
    // m_MissionReader.GetConfigurationParam("Name", <string>);
    // m_Comms.Register("VARNAME", 0);

    RegisterVariables();
    return (true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool AcommsSimulator::Iterate() {
    return (true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool AcommsSimulator::OnStartUp() {
    return (true);
}

//---------------------------------------------------------
// Procedure: RegisterVariables

void AcommsSimulator::RegisterVariables() {
    // m_Comms.Register("FOOBAR", 0);
    m_Comms.Register("ACOMMS_SIM_REPORT", 0);
    m_Comms.Register("ACOMMS_SIM_IN", 0);
}

void AcommsSimulator::handleReport(const AcommsSimReport &asr) {
    // check if we already know about this vehicle
    string vname = asr.vehicle_name();
    m_vehicleStatus[vname] = asr;
}

void AcommsSimulator::handleNewTransmission(
        const goby::acomms::protobuf::ModemTransmission & trans,
        std::string source_vehicle) {

    // check that channel is available
    if (m_channelState != AVAILABLE) {
        publishWarning(
                "Channel unavailable when trying to transmit: "
                        + trans.DebugString());
        return;
    }
}

void AcommsSimulator::publishWarning(string msg) {

}
