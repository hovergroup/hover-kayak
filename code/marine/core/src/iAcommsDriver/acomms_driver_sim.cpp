/*
 * iAcommsDriver
 *        File: acomms_driver_sim.cpp
 *  Created on: Jan 26, 2015
 *      Author: Josh Leighton
 */

#include <iterator>
#include <sstream>
#include <algorithm>
#include "acomms_driver.h"

using namespace std;

// static call back functions for simulation

static bool OnSimMailCallback(void * pParam) {
    acomms_driver * self = static_cast<acomms_driver*>(pParam);
    self->OnSimMail(pParam);
}

static bool OnSimConnectCallback(void * pParam) {
    acomms_driver * self = static_cast<acomms_driver*>(pParam);
    self->OnSimConnect(pParam);
}

void acomms_driver::simIterate() {
    // simulation reports
    if (MOOSTime() - m_lastSimReportTime > 1) {
        publishSimReport();
    }

    // simulation version
    goby::acomms::protobuf::ModemTransmission tmp;
    goby::acomms::protobuf::ModemRaw raw_msg;
    bool new_reception = false;
    bool new_raw = false;

    // check for new messages
    m_simReceiveMutex.lock();
    if (m_newSimReception) {
        tmp.CopyFrom(m_simReception);
        m_newSimReception = false;
        new_reception = true;
    }
    if (m_newSimRaw) {
        new_raw = true;
        raw_msg.set_raw(m_simRaw);
    }
    m_simReceiveMutex.unlock();

    // handle raw messages first
    if (new_raw) {
        handle_raw_incoming(raw_msg);
    }

    // handle incoming receptions
    if (new_reception) {
        handle_data_receive(tmp);
    }
}

// on connect to local
bool acomms_driver::SimConnect() {
    // get info from process config
    bool ok1, ok2;
    std::string sim_server;
    int sim_port;
    ok1 = m_MissionReader.GetConfigurationParam("sim_server", sim_server);
    ok2 = m_MissionReader.GetConfigurationParam("sim_port", sim_port);
    if (!ok1 || !ok2) {
        std::cout
                << "Simulation server and port not specified in process config"
                << std::endl;
        return false;
    }

    // configure reports
    m_simReport.set_vehicle_name(my_name);
    m_simReport.set_ranging_enabled(enable_one_way_ranging);

    // construct app name
    std::string app_name = my_name + "_" + GetAppName();

    // set callbacks and run
    sim_Comms.SetOnConnectCallBack(OnSimConnectCallback, this);
    sim_Comms.SetOnMailCallBack(OnSimMailCallback, this);
    sim_Comms.Run(sim_server, sim_port, app_name);
}

// on connect to shoreside (simulation only)
bool acomms_driver::OnSimConnect(void * pParam) {
    // construct name to register for incoming acomms
    std::string caps_name = MOOSToUpper(my_name.c_str());
    m_simReceiveVarName = "ACOMMS_SIM_OUT_" + caps_name;

    // register
    sim_Comms.Register(m_simReceiveVarName, 0);
    return true;
}

// on mail from shoreside (simulation only)
bool acomms_driver::OnSimMail(void * pParam) {
    // fetch msg list
    MOOSMSG_LIST M;
    sim_Comms.Fetch(M);

    // iterate over elements
    MOOSMSG_LIST::iterator q;
    for (q = M.begin(); q != M.end(); q++) {
        std::string key = q->GetKey();

        // complete receptions
        if (key == m_simReceiveVarName) {
            std::string msg = q->GetString();
            goby::acomms::protobuf::ModemTransmission tmp;

            // got completed reception
            if (tmp.ParseFromString(msg)) {
                m_simReceiveMutex.lock();
                m_simReception.Clear();
                m_simReception.CopyFrom(tmp);
                m_newSimReception = true;
                m_simReceiveMutex.unlock();
            }

            // got raw data - start of transmission
            else {
                m_simReceiveMutex.lock();
                m_newSimRaw = true;
                m_simRaw = msg;
                m_simReceiveMutex.unlock();
            }
        }
    }

    return true;
}

void acomms_driver::publishSimReport() {
    m_simReport.set_x(m_navx);
    m_simReport.set_y(m_navy);

    std::string out = m_simReport.SerializeAsString();
    if (!out.empty())
        sim_Comms.Notify("ACOMMS_SIM_REPORT", (void*) out.data(), out.size());

    out = m_simReport.DebugString();
    if (!out.empty())
        sim_Comms.Notify("ACOMMS_SIM_REPORT_DEBUG", out);
}
