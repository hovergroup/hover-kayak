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
    m_state = READY;
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

            if (m_postQueue.empty())
                break;
        }
    }
}


//---------------------------------------------------------
// Constructor

AcommsSimulator::AcommsSimulator() {
    // state variables
    m_channelState = AVAILABLE;

    SPEED_OF_SOUND = 1450;
    P_PARTIAL_LOSS = 0.1;
    P_COMPLETE_LOSS = 0.1;
    P_SYNC_LOSS = 0.1;
    P_NO_LOSS = 1 - P_PARTIAL_LOSS - P_COMPLETE_LOSS - P_SYNC_LOSS;
    P_LOSS_PER_FRAME = 0.5;
    if (P_PARTIAL_LOSS < 0 || P_COMPLETE_LOSS < 0 || P_SYNC_LOSS < 0 || P_NO_LOSS < 0 ||
            P_LOSS_PER_FRAME < 0 || P_PARTIAL_LOSS > 1 || P_COMPLETE_LOSS > 1 ||
            P_SYNC_LOSS > 1 || P_NO_LOSS > 1 || P_LOSS_PER_FRAME < 0 || P_LOSS_PER_FRAME > 1) {
        cout << "Default losses in constructor were impossible." << endl;
        exit(1);
    }

    srand(time(NULL));

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
        std::string key = msg.GetKey();

        // report updates
        if (key == "ACOMMS_SIM_REPORT") {
            AcommsSimReport asr;
            if (asr.ParseFromString(msg.GetString())) {
                handleReport(asr);
            } else
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
                    cout << "Handling new transmission from " << source_vehicle << endl;
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
    for (int i=0; i<m_vehicles.size(); i++) {
        m_singleSims[m_vehicles[i]].doWork();
    }
    return (true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool AcommsSimulator::OnStartUp() {
    m_MissionReader.GetConfigurationParam("sound_speed", SPEED_OF_SOUND);
    m_MissionReader.GetConfigurationParam("p_partial_loss", P_PARTIAL_LOSS);
    m_MissionReader.GetConfigurationParam("p_complete_loss", P_COMPLETE_LOSS);
    m_MissionReader.GetConfigurationParam("p_sync_loss", P_SYNC_LOSS);
    m_MissionReader.GetConfigurationParam("p_loss_per_frame", P_LOSS_PER_FRAME);

    P_NO_LOSS = 1 - P_PARTIAL_LOSS - P_COMPLETE_LOSS - P_SYNC_LOSS;
    if (P_PARTIAL_LOSS < 0 || P_COMPLETE_LOSS < 0 || P_SYNC_LOSS < 0 || P_NO_LOSS < 0 ||
            P_LOSS_PER_FRAME < 0 || P_PARTIAL_LOSS > 1 || P_COMPLETE_LOSS > 1 ||
            P_SYNC_LOSS > 1 || P_NO_LOSS > 1 || P_LOSS_PER_FRAME < 0 || P_LOSS_PER_FRAME > 1) {
        cout << "Bad loss configuration" << endl;
        return false;
    }

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
    string name = MOOSToUpper(asr.vehicle_name());
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

    // abort existing reception (handles conflicts)
    if (m_singleSims[source_vehicle].getState() != READY) {
        cout << source_vehicle << " was in state " << m_singleSims[source_vehicle].getState() <<
                ", clearing queue for new transmission." << endl;
        m_singleSims[source_vehicle].clearQueue();
    }

    // check that channel is available
    if (m_channelState != AVAILABLE) {
        cout << "Channel unavailable for new transmission from " << source_vehicle << endl;
    } else {
        // if the transmitter is not time synced, create a random offset for this transmission
        if (!m_singleSims[source_vehicle].getRangingEnabled()) {
            m_randomTimeOffset = randZeroToOne();
        }

        double start_time = MOOSTime();
        for (int i=0; i<m_vehicles.size(); i++) {
            string dest_vehicle = m_vehicles[i];
            // don't create reception for transmitter
            if (dest_vehicle != source_vehicle) {
                // create a reception
                goby::acomms::protobuf::ModemTransmission reception;
                createReception(trans, reception, source_vehicle, dest_vehicle);

                // apply losses
                LossType loss = calculateLoss(reception, source_vehicle, dest_vehicle);

                // calculate time delay and start reception (if not sync loss)
                double time_of_flight = getTimeOfFlight(source_vehicle, dest_vehicle);
                if (loss != SYNC) {
                    m_singleSims[dest_vehicle].startReception(start_time + time_of_flight, reception);
                }
            }
        }
    }

    // always start transmission on source vehicle
    m_singleSims[source_vehicle].startTransmission(trans);
}

double AcommsSimulator::getTimeOfFlight(std::string source_vehicle, std::string dest_vehicle) {
    double source_x = m_singleSims[source_vehicle].getX();
    double source_y = m_singleSims[source_vehicle].getY();
    double source_depth = m_singleSims[source_vehicle].getDepth();
    double dest_x = m_singleSims[dest_vehicle].getX();
    double dest_y = m_singleSims[dest_vehicle].getY();
    double dest_depth = m_singleSims[dest_vehicle].getDepth();

    double horizontal = sqrt(pow(dest_x-source_x,2.0) + pow(dest_y-source_y,2.0));
    double distance = sqrt(pow(horizontal,2.0) + pow(dest_depth-source_depth,2.0));
    return distance / SPEED_OF_SOUND;
}



/*
 * frame with bad crc extension
 * ranging reply extension
 * number of receive statistics
 * # of frames in receive stat should be correct
 */
void AcommsSimulator::createReception(const goby::acomms::protobuf::ModemTransmission & transmission,
           goby::acomms::protobuf::ModemTransmission & reception,
            std::string source_vehicle,
            std::string dest_vehicle) {

    reception.CopyFrom(transmission);

    // add extra stat for fsk0
    int stat_index = 0;
    if (reception.rate() == 0) {
        reception.AddExtension(micromodem::protobuf::receive_stat);
        micromodem::protobuf::ReceiveStatistics fsk_mini_stat = getCompleteStat();
        reception.MutableExtension(micromodem::protobuf::receive_stat, 0)->CopyFrom(fsk_mini_stat);
        stat_index = 1;
    }
    reception.AddExtension(micromodem::protobuf::receive_stat);

    // add main statistics
    micromodem::protobuf::ReceiveStatistics main_stat = getCompleteStat();
    if (transmission.type() == goby::acomms::protobuf::ModemTransmission::DRIVER_SPECIFIC) {
        if (transmission.GetExtension(micromodem::protobuf::type)==micromodem::protobuf::MICROMODEM_REMUS_LBL_RANGING) {
            // lbl
        } else {
            // mini
        }
    } else {
        main_stat.set_rate(transmission.rate());
    }
    main_stat.set_number_frames(reception.frame_size());
    main_stat.set_number_bad_frames(0);
    main_stat.set_source(transmission.src());
    main_stat.set_dest(transmission.dest());
    reception.MutableExtension(micromodem::protobuf::receive_stat, stat_index)->CopyFrom(main_stat);

    bool source_ranging = m_singleSims[source_vehicle].getRangingEnabled();
    bool dest_ranging = m_singleSims[dest_vehicle].getRangingEnabled();
    if (source_ranging && dest_ranging) {
        // ranging enabled on both - get actual time of flight
        micromodem::protobuf::RangingReply rr;
        rr.add_one_way_travel_time(getTimeOfFlight(source_vehicle, dest_vehicle));
        reception.MutableExtension(micromodem::protobuf::ranging_reply)->CopyFrom(rr);
    } else if (!source_ranging && dest_ranging) {
        // ranging enabled on dest only - add random offset
        micromodem::protobuf::RangingReply rr;
        double time_of_flight = getTimeOfFlight(source_vehicle, dest_vehicle) + m_randomTimeOffset;
        while (time_of_flight > 1) {
            time_of_flight-=1;
        }
        rr.add_one_way_travel_time(time_of_flight);
        reception.MutableExtension(micromodem::protobuf::ranging_reply)->CopyFrom(rr);
    }
}

micromodem::protobuf::ReceiveStatistics AcommsSimulator::getCompleteStat() {
    micromodem::protobuf::ReceiveStatistics stat;
    stat.set_mode(micromodem::protobuf::INVALID_RECEIVE_MODE);
    stat.set_time(MOOSTime());
    stat.set_clock_mode(micromodem::protobuf::INVALID_CLOCK_MODE);
    stat.set_mfd_power(-1);
    stat.set_mfd_ratio(-1);
    stat.set_rate(-1);
    stat.set_source(-1);
    stat.set_dest(-1);
    stat.set_psk_error_code(micromodem::protobuf::INVALID_PSK_ERROR_CODE);
    stat.set_packet_type(micromodem::protobuf::PACKET_TYPE_UNKNOWN);
    stat.set_number_frames(-1);
    stat.set_number_bad_frames(-1);
    stat.set_snr_rss(-1);
    stat.set_snr_in(-1);
    stat.set_snr_out(-1);
    stat.set_snr_symbols(-1);
    stat.set_mse_equalizer(-1);
    stat.set_data_quality_factor(-1);
    stat.set_doppler(-1);
    stat.set_stddev_noise(-1);
    stat.set_carrier_freq(-1);
    stat.set_bandwidth(-1);
    return stat;
}

LossType AcommsSimulator::calculateLoss(goby::acomms::protobuf::ModemTransmission & reception,
            std::string source_vehicle,
            std::string dest_vehicle) {

    // determine loss type
    double rand = randZeroToOne();
    LossType loss;
    if (rand < P_NO_LOSS) {
        loss = NONE;
    } else if (rand < P_NO_LOSS + P_PARTIAL_LOSS) {
        loss = PARTIAL;
    } else if (rand < P_NO_LOSS + P_PARTIAL_LOSS + P_COMPLETE_LOSS) {
        loss = COMPLETE;
    } else {
        loss = SYNC;
    }

    // implement selected loss type
    switch (loss) {
    case NONE:
        cout << "No loss from " << source_vehicle << " to " << dest_vehicle << endl;
        // do nothing
        return NONE;
        break;

    case PARTIAL:
        cout << "Partial loss from " << source_vehicle << " to " << dest_vehicle << endl;
        applyPartialLoss(reception);
        return PARTIAL;
        break;

    case COMPLETE:
        cout << "Complete loss from " << source_vehicle << " to " << dest_vehicle << endl;
        applyCompleteLoss(reception);
        return COMPLETE;
        break;

    case SYNC:
        cout << "Sync loss from " << source_vehicle << " to " << dest_vehicle << endl;
        // whole transmission missed, so no need for action
        return SYNC;
        break;
    }

}

void AcommsSimulator::applyPartialLoss(goby::acomms::protobuf::ModemTransmission & reception) {
    // iterate over frames and apply p_loss_per_farme_individually
    int num_bad = 0;
    int num_frames = reception.frame_size();
    vector<bool> bad_frames (num_frames, false);
    for (int i=0; i<num_frames; i++) {
        double rand = randZeroToOne();
        if (rand < P_LOSS_PER_FRAME) {
            bad_frames[i] = true;
            num_bad++;
        }
    }

    // if no bad frames, randomly select one to be bad
    if (num_bad == 0) {
        double rand = randZeroToOne() * num_frames;
        for (int i=0; i<num_frames; i++) {
            if (rand < i+1) {
                bad_frames[i] = true;
                break;
            }
        }
    }

    // if all bad frames, randomly select one to be good
    if (num_bad == num_frames) {
        double rand = randZeroToOne() * num_frames;
        for (int i=0; i<num_frames; i++) {
            if (rand < i+1) {
                bad_frames[i] = false;
                break;
            }
        }
    }

    // implement bad frames
    for (int i=0; i<num_frames; i++) {
        if (bad_frames[i]) {
            reception.set_frame(i,"");
            reception.AddExtension(micromodem::protobuf::frame_with_bad_crc, i);
        }
    }

    int index = reception.ExtensionSize(micromodem::protobuf::receive_stat)-1;
    reception.MutableExtension(micromodem::protobuf::receive_stat, index)->set_number_bad_frames(num_bad);

    cout << "Bad frames: ";
    for (int i=0; i<num_frames; i++) {
        if (bad_frames[i])
            cout << i << " ";
    }
    cout << endl;
}

void AcommsSimulator::applyCompleteLoss(goby::acomms::protobuf::ModemTransmission & reception) {
    int num_frames = reception.frame_size();
    for (int i=0; i<num_frames; i++) {
        reception.set_frame(i,"");
        reception.AddExtension(micromodem::protobuf::frame_with_bad_crc, i);
    }

    int index = reception.ExtensionSize(micromodem::protobuf::receive_stat)-1;
    reception.MutableExtension(micromodem::protobuf::receive_stat, index)->set_number_bad_frames(num_frames);
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

double AcommsSimulator::randZeroToOne() {
    return ((double) rand()) / ((double) RAND_MAX);
}
