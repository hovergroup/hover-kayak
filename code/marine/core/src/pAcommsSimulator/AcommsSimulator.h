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
#include <deque>
#include "boost/assign.hpp"

enum ChannelState {
    AVAILABLE, BUSY
};

enum DriverStatus {
	TRANSMIT_POSTED,
	TRANSMIT_STARTED,
	RECEIVING_SOUND,
	RECEIVING_PARSE,
	READY
};

static const std::map<DriverStatus,std::string> DriverStatusNameMap =
		boost::assign::map_list_of
		(TRANSMIT_POSTED,"TRANSMIT_POSTED")
		(TRANSMIT_STARTED,"TRANSMIT_STARTED")
		(RECEIVING_SOUND,"RECEIVING_SOUND")
		(RECEIVING_PARSE,"RECEIVING_PARSE")
		(READY,"READY");

// from db post to channel active
static const double POST_TO_TRANSMIT_DELAY = 0;
// from db post to TXF
static const double MINI_TRANSMIT_LENGTH = 2;
static const double FSK_TRANSMIT_LENGTH = 5;
static const double PSK1_TRANSMIT_LENGTH = 5;
static const double PSK2_TRANSMIT_LENGTH = 5;
static const double PARSE_TIME = 0.5;
// map for easy access - populated in constructor
extern std::map<int,double> RATE_TRANSMIT_LENGTH_MAP;

class SingleDriverSim {
public:
	struct PostEvent {
		double post_time;
		DriverStatus new_state;
		std::string to_post;
	};

public:
	SingleDriverSim(std::string name, MOOS::MOOSAsyncCommClient * comms);
	SingleDriverSim() {}
	~SingleDriverSim() {}

	bool startTransmission(goby::acomms::protobuf::ModemTransmission transmission);
	bool startReception(double receive_start_time,
			goby::acomms::protobuf::ModemTransmission reception);
	bool updateWithReport(const AcommsSimReport &asr);
	void clearQueue();
	void doWork();
	double getTransmitLength(goby::acomms::protobuf::ModemTransmission transmission);

	DriverStatus getState() { return m_state; }
	std::string getName() { return m_name; }

	double getX() { return m_navx; }
	double getY() { return m_navy; }
	double getDepth() { return m_navdepth; }
	double getHeading() { return m_navheading; }
	double getSpeed() { return m_navspeed; }
	bool getRangingEnabled() { return m_rangingEnabled; }

private:
	DriverStatus m_state;

	std::string m_name, m_postVariable;
	double m_navx, m_navy, m_navdepth, m_navheading, m_navspeed;
	bool m_rangingEnabled;

	double m_queueStartTime;
	std::deque<PostEvent> m_postQueue;
	MOOS::MOOSAsyncCommClient * m_Comms;
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
    std::map<std::string, SingleDriverSim> m_singleSims;
    std::vector<std::string> m_vehicles;

    bool vehicleExists(std::string name);

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
