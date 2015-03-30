/*
 * pAckedCommsShoreside
 *        File: pAckedCommsShoreside.h
 *  Created on: May 24, 2014
 *      Author: Josh Leighton
 */

#ifndef AckedCommsShoreside_HEADER
#define AckedCommsShoreside_HEADER

#include "MOOS/libMOOS/MOOSLib.h"
#include <vector>
#include <map>

class AckedCommsShoreside: public CMOOSApp {
private:
    class AckedTransmission {
    public:
        AckedTransmission() {
            transmit_time = -1;
            num_transmits = 0;
        }

        enum TypeEnum {
            DOUBLE,
            STRING,
            BINARY_STRING
        };

        // what we're sending
        TypeEnum type;
        std::string var;
        std::string string_val;
        double double_val;
        unsigned int id;

        // where we're sending
        std::string destination;

        // how we're sending
        double delay;
        unsigned int repeat;

        // state information
        double transmit_time;
        unsigned int num_transmits;
    };

public:
    AckedCommsShoreside();
    ~AckedCommsShoreside();

protected:
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();
    void RegisterVariables();

    void printFormattedTransmission(AckedTransmission & trans);

private:
    bool m_started;

    // variables we're subbing too and associated configuration
    std::vector<std::string> m_vars, m_vehicles;
    std::map<std::string, double> m_repeats, m_delays;

    // id for tracking transmissions
    unsigned int m_currentID;

    // active transmissions
    std::vector<AckedTransmission> m_transmissions;
};

#endif 
