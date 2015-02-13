/*
 * pAcommsSimulator
 *        File: AcommsSimulator.cpp
 *  Created on: Jan 13, 2014
 *      Author: Josh Leighton
 */

/* Description of Operation
 *
 * The operation of the AcommsSimulator can be split into these primary functions:
 *    1 - Tracking of active acomms endpoints (vehicles)
 *    2 - Conversion of transmissions to receptions
 *    3 - Implementation of packet loss
 *    4 - Timing of transmissions and receptions
 *    5 - Handling of conflicting transmissions
 *
 * 1 - TRACKING ACTIVE VEHICLES
 *     Acomms drivers (vehicle endpoints) periodically post reports to the ACOMMS_SIM_REPORT
 * variable using the AcommsSimReport protobuf.  These reports contain basic nav information
 * and some acomms information for each vehicle.  AcommsSimulator stores up to date
 * information for each vehicle using the SingleDriverSim class.
 *
 * 2 - CONVERSION OF TRANSMISSIONS TO RECEPTIONS
 *     New transmissions are sent to the driver using the ACOMMS_SIM_IN variable.  The
 * format is $VNAME,ModemTranmissionProtobuf.  The first step is to convert the transmission
 * to a reception for each active vehicle.  This is done in the main application.
 *
 * 3 - IMPLEMENTATION OF PACKET LOSS
 *     The newly created receptions and then subjected to packet loss.  This may involve
 * further editing of the reception, for example the removal of some or all frame data.
 * This step is also performed by the main application.
 *
 * 4 - TIMING OF TRANSMISSIONS AND RECEPTIONS
 *     The initial transmission and converted receptions are passed to the SingleDriverSim
 * objects.  Timing is handled by these objects inside the doWork() function, which is
 * called during every iterate of the main application.  Postings to the vehicles' drivers
 * is done within this function.
 *
 * 5 - CONFLICTING TRANSMISSIONS
 *     a - Simultaneous transmissions occur when a second driver initiates a transmission
 *         before it receives a receive start signal.  In this case, the simulator will
 *         retroactively cancel the reception for that driver and instead execute the
 *         timing for a transmission on that driver.  No other drivers will hear its
 *         transmission though.
 *
 *     b - It is possible, though unlikely, for a driver to initiate a transmission while
 *         the receive start signal is already waiting to be read by its second mail
 *         client.  For this reason, the driver will check it state when receiving the
 *         receive start signal and ignore it if it is not in the "ready" state.  The
 *         simulator will handle this as a simultaneous transmission, canceling the
 *         reception and running the transmission timings.
 *
 */

#include <iterator>
#include "MBUtils.h"
#include "AcommsSimulator.h"

using namespace std;

std::map<int,double> RATE_TRANSMIT_LENGTH_MAP;

SingleDriverSim::SingleDriverSim(string name, MOOS::MOOSAsyncCommClient * comms) {
	m_Comms = comms;
	m_name = MOOSToUpper(name.c_str());
	m_navdepth = 0;
	m_navheading = 0;
	m_navspeed = 0;
	m_postVariable = "ACOMMS_SIM_OUT_" + m_name;
}

bool SingleDriverSim::updateWithReport(const AcommsSimReport &asr) {
	if (asr.vehicle_name() != m_name) {
		cout << "Name mismatch: " << asr.vehicle_name() << " != " <<
				m_name << " - not updating." << endl;
		return false;
	}
	m_navx = asr.x();
	m_navy = asr.y();
	if (asr.has_depth())
		m_navdepth = asr.depth();
	if (asr.has_heading())
		m_navheading = asr.heading();
	if (asr.has_speed())
		m_navspeed = asr.speed();
	m_rangingEnabled = asr.ranging_enabled();

	return true;
}

double SingleDriverSim::getTransmitLength(goby::acomms::protobuf::ModemTransmission transmission) {
	if (transmission.type() == goby::acomms::protobuf::ModemTransmission::DRIVER_SPECIFIC) {
		if (transmission.GetExtension(micromodem::protobuf::type)==micromodem::protobuf::MICROMODEM_REMUS_LBL_RANGING) {
			return 1;
		} else {
			return MINI_TRANSMIT_LENGTH;
		}
	} else
		return RATE_TRANSMIT_LENGTH_MAP[transmission.rate()];
}

bool SingleDriverSim::startTransmission(goby::acomms::protobuf::ModemTransmission transmission) {
	if (m_state != READY) {
		cout << m_name << " failed to start transmission while in state "
				<< DriverStatusNameMap.at(m_state) << endl;
		return false;
	}

	m_queueStartTime = MOOSTime();

	PostEvent transmit_posted;
	transmit_posted.post_time = MOOSTime();
	transmit_posted.new_state = TRANSMIT_POSTED;
	transmit_posted.to_post = "";
	m_postQueue.push_back(transmit_posted);

	PostEvent transmit_started;
	transmit_started.post_time = transmit_posted.post_time + POST_TO_TRANSMIT_DELAY;
	transmit_started.new_state = TRANSMIT_STARTED;
	transmit_started.to_post = "";
	m_postQueue.push_back(transmit_started);

	PostEvent transmit_done;
	transmit_done.post_time = transmit_started.post_time + getTransmitLength(transmission);
	transmit_done.new_state = READY;
	transmit_done.to_post = "TXF";
	m_postQueue.push_back(transmit_done);

	return true;
}

bool SingleDriverSim::startReception(double receive_start_time,
		goby::acomms::protobuf::ModemTransmission reception) {
	if (m_state != READY) {
		cout << m_name << " failed to start reception while in state "
				<< DriverStatusNameMap.at(m_state) << endl;
		return false;
	}

	m_queueStartTime = receive_start_time;

	PostEvent receive_start;
	receive_start.post_time = receive_start_time;
	receive_start.new_state = RECEIVING_SOUND;
	receive_start.to_post = "RXP";
	m_postQueue.push_back(receive_start);

	PostEvent receive_parse;
	receive_parse.post_time = receive_start_time + getTransmitLength(reception);
	receive_parse.new_state = RECEIVING_PARSE;
	receive_parse.to_post = "";
	m_postQueue.push_back(receive_parse);

	PostEvent receive_done;
	receive_done.post_time = receive_parse.post_time + PARSE_TIME;
	receive_done.new_state = READY;
	receive_done.to_post = reception.SerializeAsString();
	m_postQueue.push_back(receive_done);

	return true;
}
void SingleDriverSim::clearQueue() {
	m_postQueue.clear();
	m_state = READY;
}

void SingleDriverSim::doWork() {
	if (!m_postQueue.empty()) {
		while (MOOSTime() > m_postQueue.front().post_time) {
			if (!m_postQueue.front().to_post.empty()) {
				m_Comms->Notify(m_postVariable,
						(void *) m_postQueue.front().to_post.data(),
						m_postQueue.front().to_post.size());
			}
			m_state = m_postQueue.front().new_state;
			cout << m_name << " advanced to state " << DriverStatusNameMap.at(m_state)
					<< " after " << m_postQueue.front().post_time-m_queueStartTime << " seconds." << endl;
			m_postQueue.pop_front();
		}
	}
}


//---------------------------------------------------------
// Constructor

AcommsSimulator::AcommsSimulator() {
    // state variables
    m_channelState = AVAILABLE;

    RATE_TRANSMIT_LENGTH_MAP[0] = FSK_TRANSMIT_LENGTH;
    RATE_TRANSMIT_LENGTH_MAP[1] = PSK1_TRANSMIT_LENGTH;
    RATE_TRANSMIT_LENGTH_MAP[2] = PSK2_TRANSMIT_LENGTH;
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
                std::string source_vehicle = MOOSToUpper(MOOSChomp(app_name, "_"));

                // check that we have reports from source vehicle
                if (!vehicleExists(source_vehicle)) {
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
	string name = asr.vehicle_name();
    // check if we already know about this vehicle
	if (vehicleExists(name)) {
		m_singleSims[name].updateWithReport(asr);
	} else {
		m_vehicles.push_back(name);
		SingleDriverSim newsim(name, &m_Comms);
		m_singleSims[name] = newsim;
		m_singleSims[name].updateWithReport(asr);
	}
    string vname = asr.vehicle_name();
}

void AcommsSimulator::handleNewTransmission(
        const goby::acomms::protobuf::ModemTransmission & trans,
        std::string source_vehicle) {

	if (m_singleSims[source_vehicle].getState() != READY) {
		cout << source_vehicle << " was in state " << m_singleSims[source_vehicle].getState() <<
				", clearing queue for new transmission." << endl;
		m_singleSims[source_vehicle].clearQueue();
	}

    // check that channel is available
    if (m_channelState != AVAILABLE) {
    	cout << "Channel unavailable for new transmission from " << source_vehicle << endl;
    	m_singleSims[source_vehicle].startTransmission(trans);
        return;
    } else {
    	for (int i=0; i<m_vehicles.size(); i++) {
    		if (m_vehicles[i] != source_vehicle) {

    		}
    	}
    }
}

void AcommsSimulator::publishWarning(string msg) {

}

bool AcommsSimulator::vehicleExists(string name) {
	map<string, SingleDriverSim>::iterator it;
	it = m_singleSims.find(name);
	if (it == m_singleSims.end())
		return false;
	else
		return true;
}
