/*
 * pAcommsSimulator
 *        File: AcommsSimulator.h
 *  Created on: Jan 13, 2014
 *      Author: Josh Leighton
 */

#ifndef AcommsSimulator_HEADER
#define AcommsSimulator_HEADER

#include "MOOS/libMOOS/MOOSLib.h"

#include "goby/acomms/protobuf/mm_driver.pb.h"

#include "acommsSim.pb.h"

#include <map>

enum ChannelState {
    AVAILABLE, BUSY
};

class AcommsSimulator: public CMOOSApp {
public:
    AcommsSimulator();
    ~AcommsSimulator();

protected:
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();
    void RegisterVariables();

private:
    std::map<std::string, AcommsSimReport> m_vehicleStatus;

    // data handling
    void handleReport(const AcommsSimReport &asr);
    void handleNewTransmission(
            const goby::acomms::protobuf::ModemTransmission & trans,
            std::string source_vehicle);

    // state variables
    ChannelState m_channelState;

    // utility
    void publishWarning(std::string msg);
};

#endif 
