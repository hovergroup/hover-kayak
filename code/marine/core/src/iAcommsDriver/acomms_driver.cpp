/*
 * iAcommsDriver
 *        File: acomms_driver.cpp
 *  Created on: May 24, 2012
 *      Author: Josh Leighton
 */

#include <iterator>
#include <sstream>
#include <algorithm>
#include "acomms_driver.h"

using namespace std;

//---------------------------------------------------------
// Constructor

acomms_driver::acomms_driver() {
    // state variables
    m_transmitLockout = false;
    m_status = HoverAcomms::NOT_RUNNING;
    m_newSimReception = false;
    m_newSimRaw = false;

    // port info
    port_name = "/dev/ttyUSB0";
    my_name = "default_name";

    // time keeping
    receive_set_time = 0;
    status_set_time = 0;
    start_time = -1;
    m_lastSimReportTime = -1;

    // configuration
    use_psk_for_minipackets = false;
    enable_one_way_ranging = false;
    enable_range_pulses = true;
    in_sim = false;
    m_useScheduler = false;

    // default (empty) transmission protobuf
    m_transmission.setDest(0);
    m_transmission.setRate(0);
    m_transmission.fillData("default_data");
    m_transmission.m_protobuf.set_ack_requested(false);
}

//---------------------------------------------------------
// Destructor

acomms_driver::~acomms_driver() {
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool acomms_driver::OnNewMail(MOOSMSG_LIST &NewMail) {
    MOOSMSG_LIST::iterator p;
    bool new_transmit = false;

    for (p = NewMail.begin(); p != NewMail.end(); p++) {
        CMOOSMsg &msg = *p;
        string key = msg.GetKey();

        // set transmission rate in protobuf
        if (key == "ACOMMS_TRANSMIT_RATE") {
            if (!m_transmission.setRate(msg.GetDouble())) {
                publishWarning("Transmit rate not supported.");
            }
        }

        // set destination address
        else if (key == "ACOMMS_TRANSMIT_DEST") {
            m_transmission.setDest(msg.GetDouble());
        }

        // fill in data and signal transmission
        else if ((key == "ACOMMS_TRANSMIT_DATA" ||    // normal variable
                key == "SCHEDULER_TRANSMIT_DATA") &&   // using scheduler
                msg.GetSource() != GetAppName()) {  // not from this app
            if (m_transmission.fillData(msg.GetString()) == -1) {
                publishWarning("Cannot fill data because rate is not defined.");
            } else {
                new_transmit = true;
            }
        }

        // fill in binary data and signal transmission
        else if ((key == "ACOMMS_TRANSMIT_DATA_BINARY"
                || key == "SCHEDULER_TRANSMIT_DATA_BINARY")
                && msg.GetSource() != GetAppName()) {
            if (m_transmission.fillData(msg.GetString()) == -1) {
                publishWarning("Cannot fill data because rate is not defined.");
            } else {
                new_transmit = true;
            }
        }

        // grab logger directory from pLogger
        else if (key == "LOGGER_DIRECTORY"
                && m_status == HoverAcomms::NOT_RUNNING) {
            // get log directory from plogger that we will also use
            startDriver(msg.GetString());
        }

        // update x or y location
        else if (key == "NAV_X") {
            m_navx = msg.GetDouble();
        } else if (key == "NAV_Y") {
            m_navy = msg.GetDouble();
        }

        // transmission from a pre-built protobof
        else if ((key == "ACOMMS_TRANSMIT" || key == "SCHEDULER_TRANSMIT")
                && msg.GetSource() != GetAppName()) {
            if (m_transmission.parseFromString(msg.GetString()))
                new_transmit = true;
            else
                publishWarning(
                        "Failed to parse protobuf from ACOMMS_TRANSMIT posting.");
        }

        // toggle modem's built in ack functionality
        else if (key == "ACOMMS_REQUEST_ACK") {
            if (msg.GetDouble() == 1) {
                m_transmission.setAckRequested(true);
            } else {
                m_transmission.setAckRequested(false);
            }
        }

        // lockout controlled by scheduler
        else if (key == "ACOMMS_TRANSMIT_LOCKOUT") {
            if (msg.GetDouble() == 1) {
                m_transmitLockout = true;
            } else {
                m_transmitLockout = false;
            }
        }
    }

    // send transmission after reading all variables
    if (new_transmit)
        transmit_data();

    return (true);
}

void acomms_driver::RegisterVariables() {
    m_Comms.Register("ACOMMS_TRANSMIT_RATE", 0);
    m_Comms.Register("ACOMMS_TRANSMIT_DEST", 0);
    if (!m_useScheduler) {
        m_Comms.Register("ACOMMS_TRANSMIT_DATA", 0);
        m_Comms.Register("ACOMMS_TRANSMIT_DATA_BINARY", 0);
        m_Comms.Register("ACOMMS_TRANSMIT", 0);
    } else {
        m_Comms.Register("SCHEDULER_TRANSMIT_DATA", 0);
        m_Comms.Register("SCHEDULER_TRANSMIT_DATA_BINARY", 0);
        m_Comms.Register("SCHEDULER_TRANSMIT", 0);
    }
    m_Comms.Register("LOGGER_DIRECTORY", 1);
    m_Comms.Register("NAV_X", 1);
    m_Comms.Register("NAV_Y", 1);
    m_Comms.Register("ACOMMS_REQUEST_ACK", 0);
    m_Comms.Register("ACOMMS_TRANSMIT_LOCKOUT", 0);

    if (in_sim) {
    }
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool acomms_driver::OnConnectToServer() {
    m_MissionReader.GetConfigurationParam("PortName", port_name);
    m_MissionReader.GetConfigurationParam("ID", my_id);
    m_MissionReader.GetValue("Community", my_name);
    MOOSToUpper(my_name);
    m_MissionReader.GetConfigurationParam("PSK_minipackets",
            use_psk_for_minipackets);
    m_MissionReader.GetConfigurationParam("enable_ranging",
            enable_one_way_ranging);
    m_MissionReader.GetConfigurationParam("show_range_pulses",
            enable_range_pulses);
    m_MissionReader.GetConfigurationParam("in_sim", in_sim);
    m_MissionReader.GetConfigurationParam("use_scheduler", m_useScheduler);

    // simulation shore server connection
    if (in_sim) {
    	if ( !SimConnect() ) return false;
    }

    // post these to make sure they are the correct type
    unsigned char c = 0x00;
    if (!m_useScheduler) {
        m_Comms.Notify("ACOMMS_TRANSMIT_DATA", ""); // string
        m_Comms.Notify("ACOMMS_TRANSMIT", &c, 1); // binary string
        m_Comms.Notify("ACOMMS_TRANSMIT_DATA_BINARY", &c, 1); // binary string
    }
//    m_Comms.Notify("ACOMMS_RECEIVED_DATA", &c, 1); // binary string
//    m_Comms.Notify("ACOMMS_TRANSMITTED", &c, 1);

    RegisterVariables();

    // driver not started yet
    publishStatus(HoverAcomms::NOT_RUNNING);

    return (true);
}

//---------------------------------------------------------
// Procedure: Iterate()

bool acomms_driver::Iterate() {
    // start up timer - set driver ready 5 seconds after start
    if (start_time != -1 && m_status == HoverAcomms::NOT_RUNNING) {
        if (MOOSTime() - start_time > 5) {
            publishStatus(HoverAcomms::READY);
            start_time = -1;
        }
    }

    // do nothing if driver is not initialized
    if (m_status == HoverAcomms::NOT_RUNNING
            || m_status == HoverAcomms::STARTING)
        return true;

    // run the driver
    if (!in_sim) {
        driver->do_work();
    } else {
    	simIterate();
    }

    // receive status timeout
    if (m_status == HoverAcomms::RECEIVING
            && MOOSTime() - receive_set_time > 8) {
        publishWarning("Timed out in receiving state.");
        publishStatus(HoverAcomms::READY);
    }

    // set transmit timeout based on packet type
    double transmit_timeout;
    switch (m_transmission.getRate()) {
    case HoverAcomms::MINI:
        transmit_timeout = 3;
        break;
    default:
        transmit_timeout = 8;
    }

    // check transmit timeout
    if (m_status == HoverAcomms::TRANSMITTING
            && MOOSTime() - transmit_set_time > transmit_timeout) {
        //if (!in_sim)
            publishWarning("Timed out in transmitting state.");
        publishStatus(HoverAcomms::READY);
    }

    // ensure status gets updated every 5 seconds
    if (MOOSTime() - status_set_time > 5) {
        publishStatus(m_status);
    }

    return (true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()

bool acomms_driver::OnStartUp() {
    return (true);
}

// new data transmission
void acomms_driver::transmit_data() {
    // check driver status
    if (m_status != HoverAcomms::READY) {
        publishWarning("Driver not ready");
        return;
    }

    // check transmit lockout
    if (m_transmitLockout) {
        publishWarning("Transmit lockout enabled.");
        return;
    }

    // check that we have transmission data
    if (m_transmission.getData().size() == 0
            && (m_transmission.getRate() != HoverAcomms::REMUS_LBL
                    && m_transmission.getRate() != HoverAcomms::TWO_WAY_RANGING)) {
        publishWarning("No transmission data");
        return;
    }

    // set status to transmitting
    publishStatus(HoverAcomms::TRANSMITTING);
    transmit_set_time = MOOSTime();

    // check destination
    if (m_transmission.m_protobuf.dest() < 0) {
        publishWarning("Destination not set, assuming default.");
        m_transmission.setDest(0);
    }

    // check rate
    if (m_transmission.m_protobuf.rate() < 0) {
        publishWarning("Rate not set, assuming default.");
        m_transmission.setRate(0);
    } else {
        m_transmission.setRate(m_transmission.getRate());
    }

    // set transmission source
    m_transmission.m_protobuf.set_src(my_id);

    // pull out modem transmission
    goby::acomms::protobuf::ModemTransmission trans =
            m_transmission.getProtobuf();

    if (!in_sim) {
        // pass to goby driver
        driver->handle_initiate_transmission(trans);
    } else {
        // post to shoreside variable for simulation
        std::string out = trans.SerializeAsString();
        if (!out.empty())
            sim_Comms.Notify("ACOMMS_SIM_IN", (void*) out.data(), out.size());
    }

    // post summary and hex data
    m_Comms.Notify("ACOMMS_TRANSMITTED_DATA_HEX", m_transmission.getHexData());
    m_Comms.Notify("ACOMMS_TRANSMITTED_ALL",
            m_transmission.getLoggableString());

    // post transmission range pulse
    postRangePulse(my_name + "_transmit", transmission_pulse_range,
            transmission_pulse_duration, "cyan");
}

// post range pulse for pMarineViewer
void acomms_driver::postRangePulse(string label, double range, double duration,
        string color) {
    if (!enable_range_pulses)
        return;

    XYRangePulse pulse;
    pulse.set_x(m_navx);
    pulse.set_y(m_navy);
    pulse.set_label(label);
    pulse.set_rad(range);
    pulse.set_duration(duration);
    pulse.set_time(MOOSTime());
    pulse.set_color("edge", color);
    pulse.set_color("fill", color);

    m_Comms.Notify("VIEW_RANGE_PULSE", pulse.get_spec());
}

// handle incoming data received or statistics from the modem
void acomms_driver::handle_data_receive(
        const goby::acomms::protobuf::ModemTransmission& data_msg) {
    std::cout << data_msg.DebugString() << std::endl;

    // construct reception from incoming protobuf
    HoverAcomms::AcommsReception reception;
    reception.copyFromProtobuf(data_msg);

    // verify incoming message, though currently continue regardless
    bool ok = true;
    std::string debug_msg = reception.verify(ok);
    if (!ok) {
        publishWarning(debug_msg);
    }

    // post reception in binary and loggable form
    std::string serialized = reception.serialize();
    m_Comms.Notify("ACOMMS_RECEIVED", (void*) serialized.data(),
            serialized.size());
    m_Comms.Notify("ACOMMS_RECEIVED_ALL", reception.getLoggableString());

    // post remus lbl times
    if (reception.getRate() == HoverAcomms::REMUS_LBL) {
        std::stringstream ss;
        std::vector<double> rr = reception.getRemusLBLTimes();
        for (int i = 0; i < rr.size(); i++) {
            ss << rr[i];
            if (i != rr.size() - 1)
                ss << ",";
        }
        m_Comms.Notify("REMUS_LBL_TIMES", ss.str());
    }

    // if not remus lbl, treat as normal data transmission
    else {
        // determine receive status
        HoverAcomms::ReceiptStatus status = reception.getStatus();

        m_Comms.Notify("ACOMMS_SOURCE_ID", (double) reception.getSource());
        m_Comms.Notify("ACOMMS_DEST_ID", (double) reception.getDest());
        m_Comms.Notify("ACOMMS_RATE", (double) reception.getRate());
        m_Comms.Notify("ACOMMS_ONE_WAY_TRAVEL_TIME",
                reception.getRangingTime());
        std::string data = reception.getData();
        m_Comms.Notify("ACOMMS_RECEIVED_DATA", (void*) data.data(),
                data.size());

        micromodem::protobuf::ReceiveStatistics stat =
                reception.getStatistics(1);
        m_Comms.Notify("ACOMMS_STATS_SNR_OUT", (double) stat.snr_out());
        m_Comms.Notify("ACOMMS_STATS_SNR_IN", (double) stat.snr_in());
        m_Comms.Notify("ACOMMS_STATS_DQF", (double) stat.data_quality_factor());
        m_Comms.Notify("ACOMMS_STATS_STDDEV_NOISE", (double) stat.stddev_noise());
        m_Comms.Notify("ACOMMS_STATS_MSE", (double) stat.mse_equalizer());
        m_Comms.Notify("ACOMMS_STATS_DOPPLER", (double) stat.doppler());
        m_Comms.Notify("ACOMMS_STATS_SPL", (double) stat.spl());
        m_Comms.Notify("ACOMMS_STATS_PSK_ERROR_CODE", (double) stat.psk_error_code());

        m_Comms.Notify("ACOMMS_RECEIVED_STATUS", (double) status);
        m_Comms.Notify("ACOMMS_BAD_FRAMES", reception.getBadFrameListing());

        // post hex data
        m_Comms.Notify("ACOMMS_RECEIVED_DATA_HEX", reception.getHexData());

        // post range pulse
        if (status == HoverAcomms::GOOD) {
            postRangePulse(my_name + "_receipt_good", receive_pulse_range,
                    receive_pulse_duration, "green");
        } else if (status == HoverAcomms::PARTIAL) {
            postRangePulse(my_name + "_receipt_partial", receive_pulse_range,
                    receive_pulse_duration, "yellow");
        } else {
            postRangePulse(my_name + "_receipt_bad", receive_pulse_range,
                    receive_pulse_duration, "red");
        }
    }

    publishStatus(HoverAcomms::READY);
}

void acomms_driver::handle_raw_incoming(
        const goby::acomms::protobuf::ModemRaw& msg) {
    // pull out the message descriptor
    string descriptor = msg.raw().substr(3, 3);

    // end of transmission, change status back to ready
    if (descriptor == "TXF") {
        publishStatus(HoverAcomms::READY);
    }

    // receive start, set status and flags
    else if (descriptor == "RXP") {
    	if (in_sim) {
    		if (m_status == HoverAcomms::READY) {
    	        publishStatus(HoverAcomms::RECEIVING);
    	        receive_set_time = MOOSTime();
    		} else {
    			cout << "Ignoring receive start because already transmitting" << endl;
    		}
    	} else {
			publishStatus(HoverAcomms::RECEIVING);
			receive_set_time = MOOSTime();
    	}
    }

    // impulse response, post to moosdb
    else if (descriptor == "IRE") {
        m_Comms.Notify("ACOMMS_IMPULSE_RESPONSE", msg.raw());
    }

    else if (descriptor == "ERR") {
        if (msg.raw().find("EXTSYNC timeout") != std::string::npos) {
            publishWarning("PPS sync error - transmission not sent");
        }
    }
}

// publish warning
void acomms_driver::publishWarning(std::string message) {
	cout << "WARNING: " << message << endl;
    m_Comms.Notify("ACOMMS_DRIVER_WARNING", message);
}

// publish status and update locally
void acomms_driver::publishStatus(HoverAcomms::DriverStatus status_update) {
    m_status = status_update;
    m_Comms.Notify("ACOMMS_DRIVER_STATUS", status_update);
    status_set_time = MOOSTime();

//    m_Comms.Notify("SYSTEM_TIME_SECONDS", JoshUtil::getSystemTimeSeconds());
}

// check if a file exists
bool acomms_driver::file_exists(std::string filename) {
    ifstream my_file(filename.c_str());
    if (my_file.good()) {
        my_file.close();
        return true;
    } else {
        my_file.close();
        return false;
    }
}

void acomms_driver::startDriver(std::string logDirectory) {
    publishStatus(HoverAcomms::STARTING);

    if (!in_sim) {
        cout << "opening goby log file..." << endl;

        // construct filename and increment index if filename already exists
        int file_index = 0;
        string filename = logDirectory + "/goby_log_"
                + boost::lexical_cast<string>(file_index) + ".txt";
        while (file_exists(filename)) {
            cout << filename << " already exists." << endl;
            file_index++;
            filename = logDirectory + "/goby_log_"
                    + boost::lexical_cast<string>(file_index) + ".txt";
        }

        // open logfile
        verbose_log.open(filename.c_str());
        cout << "Goby logging to " << filename << endl;

        // pass log ofstream to goby log
        goby::glog.set_name("iAcommsDriver");
        goby::glog.add_stream(goby::common::logger::DEBUG1, &std::clog);
        goby::glog.add_stream(goby::common::logger::DEBUG3, &verbose_log);

        // set serial port
        cfg.set_serial_port(port_name);
        cfg.set_modem_id(my_id);

        // construct driver
        std::cout << "Starting WHOI Micro-Modem MMDriver" << std::endl;
        driver = new goby::acomms::MMDriver;

        // set configuration variables

        // various statistics
        cfg.AddExtension(micromodem::protobuf::Config::nvram_cfg, "DQF,1");
        cfg.AddExtension(micromodem::protobuf::Config::nvram_cfg, "MFD,1");
        cfg.AddExtension(micromodem::protobuf::Config::nvram_cfg, "SHF,1");
        cfg.AddExtension(micromodem::protobuf::Config::nvram_cfg, "DOP,1");

        // impulse response
        cfg.AddExtension(micromodem::protobuf::Config::nvram_cfg, "IRE,1");

        // gain control
        cfg.AddExtension(micromodem::protobuf::Config::nvram_cfg, "AGC,0");
        cfg.AddExtension(micromodem::protobuf::Config::nvram_cfg, "AGN,250");

        // ranging
        if (enable_one_way_ranging)
            cfg.AddExtension(micromodem::protobuf::Config::nvram_cfg, "SNV,1");
        else
            cfg.AddExtension(micromodem::protobuf::Config::nvram_cfg, "SNV,0");

        // number of CTOs before hard reboot
        cfg.AddExtension(micromodem::protobuf::Config::nvram_cfg, "NRV,0");

        // psk vs. fsk minipackets
        if (use_psk_for_minipackets)
            cfg.AddExtension(micromodem::protobuf::Config::nvram_cfg, "MOD,1");
        else
            cfg.AddExtension(micromodem::protobuf::Config::nvram_cfg, "MOD,0");

        // connect receive and raw signals to our functions
        goby::acomms::connect(&driver->signal_receive,
                boost::bind(&acomms_driver::handle_data_receive, this, _1));
        goby::acomms::connect(&driver->signal_raw_incoming,
                boost::bind(&acomms_driver::handle_raw_incoming, this, _1));

        // start the driver
        driver->startup(cfg);
    }

    // record start time
    start_time = MOOSTime();

    publishStatus(HoverAcomms::READY);
}
