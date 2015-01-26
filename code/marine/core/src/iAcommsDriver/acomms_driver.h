/*
 * iAcommsDriver
 *        File: acomms_driver.h
 *  Created on: May 24, 2012
 *      Author: Josh Leighton
 */

#ifndef acomms_driver_HEADER
#define acomms_driver_HEADER

#include <boost/thread.hpp>

#include "MOOS/libMOOS/MOOSLib.h"
#include "MOOS/libMOOS/Comms/MOOSAsyncCommClient.h"

#include "goby/acomms/modem_driver.h"
#include "goby/acomms/protobuf/mm_driver.pb.h"
#include "goby/util/binary.h"
#include "goby/common/logger.h"
#include "goby/acomms/connect.h"

#include "XYRangePulse.h"

#include "acommsSim.pb.h"
#include "HoverAcomms.h"
#include "JoshUtils.h"

class acomms_driver : public CMOOSApp
{
public:
    acomms_driver();
    virtual ~acomms_driver();

    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();

protected:
    // configuration variables
    google::protobuf::uint32 my_id;
    std::string port_name, my_name;
    bool use_psk_for_minipackets, enable_one_way_ranging, enable_range_pulses;
    bool in_sim, m_useScheduler;

    // constants for range pulses
    static const double transmission_pulse_range = 30;
    static const double transmission_pulse_duration = 3;
    static const double receive_pulse_range = 20;
    static const double receive_pulse_duration = 2;

    // vehicle state information
    double m_navx, m_navy;

    // underlying driver stuff
    goby::acomms::ModemDriverBase* driver;
    goby::acomms::protobuf::DriverConfig cfg;

    void transmit_data();

    // on incoming receptions
    void handle_data_receive( const goby::acomms::protobuf::ModemTransmission& data_msg );

    // handle raw modem messages
    void handle_raw_incoming( const goby::acomms::protobuf::ModemRaw& msg );

    // application state
    HoverAcomms::DriverStatus m_status;
    bool m_transmitLockout;
    void startDriver( std::string logDirectory );
    void publishStatus(HoverAcomms::DriverStatus status_update);

    // time keeping
    double status_set_time, receive_set_time, transmit_set_time, start_time;

    // utility functions
    void publishWarning( std::string message );
    void postRangePulse( std::string label, double range, double duration, std::string color = "yellow" );
    void RegisterVariables();
    bool file_exists( std::string filename );

    std::ofstream verbose_log;

    // protobuf used for transmissions
    HoverAcomms::AcommsTransmission m_transmission;

    // simulation ------------------------------------------------------

    // comms client to connect to shoreside
    MOOS::MOOSAsyncCommClient sim_Comms;

    // sim comms connect and mail functions
public:
    bool OnSimConnect(void * pParam);
    bool OnSimMail(void * pParam);
    bool SimConnect();
    void simIterate();

protected:
    // mutex and variables for passing data into normal driver thread
    boost::mutex m_simReceiveMutex;
    goby::acomms::protobuf::ModemTransmission m_simReception;
    bool m_newSimReception, m_newSimRaw;
    std::string m_simRaw;

    // name of variable subscribed to on shoreside
    std::string m_simReceiveVarName;

    // reports sent to shoreside
    double m_lastSimReportTime;
    AcommsSimReport m_simReport;
    void publishSimReport();
};

#endif 
