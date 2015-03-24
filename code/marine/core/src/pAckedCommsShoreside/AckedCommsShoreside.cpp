/*
 * pAckedCommsShoreside
 *        File: pAckedCommsShoreside.cpp
 *  Created on: May 24, 2014
 *      Author: Josh Leighton
 */

#include <iterator>
#include <sstream>
#include "MBUtils.h"
#include "AckedCommsShoreside.h"
#include <boost/lexical_cast.hpp>
#include "ackedComms.pb.h"
#include <boost/date_time/posix_time/posix_time.hpp>

//#define DEBUG_COUT 1

using namespace std;

//---------------------------------------------------------
// Constructor

AckedCommsShoreside::AckedCommsShoreside() {
    m_started = false;
    m_currentID = 0;
}

//---------------------------------------------------------
// Destructor

AckedCommsShoreside::~AckedCommsShoreside() {
}

//---------------------------------------------------------
// Procedure: OnNewMail

void AckedCommsShoreside::printFormattedTransmission(AckedTransmission & trans) {
    stringstream dummy_duo;
    dummy_duo << trans.var + "=";
    string type;
    switch (trans.type) {
    case AckedTransmission::DOUBLE:
        dummy_duo << trans.double_val;
        type = "double";
        break;
    case AckedTransmission::STRING:
        dummy_duo << trans.string_val;
        type = "string";
        break;
    case AckedTransmission::BINARY_STRING:
        dummy_duo << "[binary data]";
        type = "binary";
        break;
    }
    string duo = dummy_duo.str();
    if (duo.size() > 30) {
        duo = duo.substr(0,27) + "...";
    }

    printf("%-30s", duo.c_str());
    printf("  %-15s", trans.destination.c_str());
    printf("  %2d", trans.num_transmits);

    boost::posix_time::ptime p(boost::posix_time::second_clock::local_time());
    unsigned long total_sec = p.time_of_day().total_seconds();
    int hours = total_sec / 3600;
    int minutes = (total_sec % 3600) / 60;
    int seconds = total_sec - 3600*hours - 60*minutes;
    printf("  %02d:%02d:%02d", hours, minutes, seconds);

}

bool AckedCommsShoreside::OnNewMail(MOOSMSG_LIST &NewMail) {
    MOOSMSG_LIST::iterator p;

    for (p = NewMail.begin(); p != NewMail.end(); p++) {
        CMOOSMsg &msg = *p;
        string key = msg.GetKey();

        if (key == "ACKEDCOMMS_RETURN_ACK") {
            unsigned int ack_id = (unsigned int) msg.GetDouble();
            stringstream debugout;
            debugout << "Got ack with id " << ack_id << endl;
            for (int i=0; i<m_transmissions.size(); i++) {
                if (ack_id == m_transmissions[i].id) {
                    debugout << "Matched ack with transmission: " << endl;
                    debugout << "    var = " << m_transmissions[i].var << endl;
                    debugout << "    dest = " << m_transmissions[i].destination << endl;
                    debugout << "    id = " << m_transmissions[i].id << endl;
                    debugout << "    attempts = " << m_transmissions[i].num_transmits << endl << endl;
#ifdef  DEBUG_COUT
                    cout << debugout.str();
#endif

                    // print success line
                    printf("\e[0;32m%-10s","SUCCESS");
                    printFormattedTransmission(m_transmissions[i]);
                    printf("\e[0m\n");

                    // remove transmission from active list
                    m_transmissions.erase(m_transmissions.begin()+i);

                    break;
                }
            }
        } else {
            // last instance of underscore separates variable from its destination
            int index = key.find_last_of('_');
            if (index != string::npos && index > 0 && index < key.length()-1) {
                // if underscore is found mid-key, separate var name and tail
                string tail = key.substr(index+1, key.length()-index-1);
                string var = key.substr(0, index);

                // look for match within our registrations
                for (int i=0; i<m_vars.size(); i++) {
                    if (var == m_vars[i]) {
                        // construct a new transmission
                        AckedTransmission new_transmission;

                        new_transmission.var = var;
                        if (msg.IsDouble()) {
                            new_transmission.double_val = msg.GetDouble();
                            new_transmission.type = AckedTransmission::DOUBLE;
                        } else if (msg.IsBinary()) {
                            new_transmission.string_val = msg.GetString();
                            new_transmission.type = AckedTransmission::BINARY_STRING;
                        } else {
                            new_transmission.string_val = msg.GetString();
                            new_transmission.type = AckedTransmission::STRING;
                        }

                        new_transmission.delay = m_delays[var];
                        new_transmission.repeat = m_repeats[var];

                        string caps_tail = tail;
                        MOOSToUpper(caps_tail);
                        if (caps_tail != "ALL") { // not broadcast, add a single transmission
                            new_transmission.destination = tail;
                            new_transmission.id = m_currentID;
                            m_currentID++;

                            m_transmissions.push_back(new_transmission);

                            stringstream debugout;
                            debugout << "NEW TRANSMISSION" << endl;
                            debugout << "    var = " << new_transmission.var << endl;
                            debugout << "    type = ";
                            switch (new_transmission.type) {
                            case AckedTransmission::DOUBLE:
                                debugout << "double" << endl;
                                break;
                            case AckedTransmission::BINARY_STRING:
                                debugout << "binary string" << endl;
                                break;
                            case AckedTransmission::STRING:
                                debugout << "string" << endl;
                                break;
                            }
                            debugout << "    dest = " << new_transmission.destination << endl;
                            debugout << "    id = " << new_transmission.id << endl << endl;
#ifdef DEBUG_COUT
                            cout << debugout.str();
#endif
                        } else { // broadcast, add a transmission for each vehicle
                            for (int j=0; j<m_vehicles.size(); j++) {
                                new_transmission.destination = m_vehicles[j];
                                new_transmission.id = m_currentID;
                                m_currentID++;

                                m_transmissions.push_back(new_transmission);

                                stringstream debugout;
                                debugout << "NEW TRANSMISSION" << endl;
                                debugout << "    var = " << new_transmission.var << endl;
                                debugout << "    type = ";
                                switch (new_transmission.type) {
                                case AckedTransmission::DOUBLE:
                                    debugout << "double" << endl;
                                    break;
                                case AckedTransmission::BINARY_STRING:
                                    debugout << "binary string" << endl;
                                    break;
                                case AckedTransmission::STRING:
                                    debugout << "string" << endl;
                                    break;
                                }
                                debugout << "    dest = " << new_transmission.destination << endl;
                                debugout << "    id = " << new_transmission.id << endl << endl;
#ifdef DEBUG_COUT
                                cout << debugout.str();
#endif
                            }
                        }
                        break;
                    }
                }

            }
        }
    }

    return (true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool AckedCommsShoreside::OnConnectToServer() {
    std::cout << "Connecting to server" << std::endl;
    if (!m_started) {
        cout << "First time connection." << endl;

        string all_vehicles;
        m_MissionReader.GetConfigurationParam("vehicles", all_vehicles);
        string vehicle = MOOSChomp(all_vehicles, ",");
        while (!vehicle.empty()) {
            m_vehicles.push_back(vehicle);
            vehicle = MOOSChomp(all_vehicles, ",");
        }

        cout << "All vehicles: ";
        for (int i=0; i<m_vehicles.size(); i++) {
            cout << m_vehicles[i] << " ";
        }
        cout << endl;

        STRING_LIST Params;
        m_MissionReader.GetConfiguration(m_sAppName, Params);

        STRING_LIST::iterator p;
        for (p = Params.begin(); p != Params.end(); p++) {
            string sParam = *p;
            string sWhat = MOOSChomp(sParam, "=");

            // look for bridge configuration lines
            if (MOOSStrCmp(sWhat, "BRIDGE")) {
                string sNewVar = stripBlankEnds(sParam);
                string var;
                double repeat = 3, delay = 0.5;

                // parse the line
                while (sNewVar.find("=") != string::npos) {
                    string key = MOOSToUpper(MOOSChomp(sNewVar, "="));
                    string val = MOOSChomp(sNewVar, ",");
                    if (key == "VAR") {
                        var = val;
                    } else if (key == "REPEAT") {
                        try {
                            repeat = boost::lexical_cast<double>(val);
                        } catch (const boost::bad_lexical_cast &) {
                            cout << "Bad config." << endl;
                            exit(1);
                        }
                    } else if (key == "DELAY") {
                        try {
                            delay = boost::lexical_cast<double>(val);
                        } catch (const boost::bad_lexical_cast &) {
                            cout << "Bad config." << endl;
                            exit(1);
                        }
                    }
                }

                // check if we already have this variable
                if (find(m_vars.begin(), m_vars.end(), var) != m_vars.end()) {
                    cout << "Ignoring repeated bridge configuration" << endl;
                } else {
                    cout << "ADDING BRIDGE: " << endl;
                    cout << "    var = " << var << "*" << endl;
                    cout << "    repeat = " << repeat << endl;
                    cout << "    delay = " << delay << endl;

                    m_vars.push_back(var);
                    m_repeats[var] = repeat;
                    m_delays[var] = delay;

                    // wildcard registration
                    m_Comms.Register(var + "*", "*", 0);
                }
            }
        }
    } else {
        cout << "Not first time connection." << endl;
    }

    m_Comms.Register("ACKEDCOMMS_RETURN_ACK");

    return true;
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool AckedCommsShoreside::Iterate() {
    for (int i=0; i<m_transmissions.size(); i++) {
        AckedTransmission & trans = m_transmissions[i];

        // send transmit if delay time has passed and more transmits can be made
        if (MOOSTime() - trans.transmit_time > trans.delay &&
                trans.num_transmits < trans.repeat) {
            string output_var = "ACKEDCOMMS_TRANSMIT_" + trans.destination;

            AckedTransmissionProto trans_proto;
            trans_proto.set_var_name(trans.var);
            switch(trans.type) {
            case AckedTransmission::DOUBLE:
                trans_proto.set_type(AckedTransmissionProto::DOUBLE);
                trans_proto.set_double_val(trans.double_val);
                break;
            case AckedTransmission::STRING:
                trans_proto.set_type(AckedTransmissionProto::STRING);
                trans_proto.set_string_val(trans.string_val);
                break;
            case AckedTransmission::BINARY_STRING:
                trans_proto.set_type(AckedTransmissionProto::BINARY_STRING);
                trans_proto.set_string_val(trans.string_val);
                break;
            }
            trans_proto.set_id(trans.id);

            string msg = trans_proto.SerializeAsString();
            m_Comms.Notify(output_var, (void*) msg.data(), msg.size());

            trans.num_transmits++;
            trans.transmit_time = MOOSTime();

#ifdef DEBUG_COUT
            cout << "Sending " << trans.var << " to " << output_var << " try " << trans.num_transmits
                    << " of " << trans.repeat << endl;
#endif
        }

        // mark transmission as failure if delay time has passed and max transmits reached
        if (MOOSTime() - trans.transmit_time > trans.delay &&
                trans.num_transmits >= trans.repeat) {

            // print failure line
            printf("\e[0;31m%-10s","FAILURE");
            printFormattedTransmission(trans);
            printf("\e[0m\n");

            m_transmissions.erase(m_transmissions.begin()+i);
        }

    }
    return (true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool AckedCommsShoreside::OnStartUp() {
    return (true);
}

//---------------------------------------------------------
// Procedure: RegisterVariables

void AckedCommsShoreside::RegisterVariables() {
// m_Comms.Register("FOOBAR", 0);
}

