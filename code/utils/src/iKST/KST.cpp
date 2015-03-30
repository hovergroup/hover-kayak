/*
 * iKST
 *        File: KST.cpp
 *  Created on: Aug 12, 2013
 *      Author: Josh Leighton
 */

#include <iterator>
#include "MBUtils.h"
#include "KST.h"

using namespace std;

//---------------------------------------------------------
// Constructor

KST::KST() {
    m_outputFilePath = "~/kst.csv";
    m_started = false;
    SetIterateMode(REGULAR_ITERATE_AND_MAIL);
    SetAppFreq(4, 0);
}

//---------------------------------------------------------
// Destructor

KST::~KST() {
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool KST::OnNewMail(MOOSMSG_LIST &NewMail) {
    std::cout << "got mail" << std::endl;
    MOOSMSG_LIST::iterator p;

    for (p = NewMail.begin(); p != NewMail.end(); p++) {
        CMOOSMsg &msg = *p;
        std::cout << msg.GetKey() << "  ";

        if (msg.IsDouble())
            m_values[msg.GetKey()] = msg.GetDouble();
        else
            m_values[msg.GetKey()] = sqrt(-1.0);
    }
    std::cout << endl << endl;

    return (true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool KST::OnConnectToServer() {
    std::cout << "Connecting to server" << std::endl;
    if (!m_started) {
        std::cout << "First time connection." << std::endl;
        m_startTime = MOOSTime();

        STRING_LIST Params;
        m_MissionReader.GetConfiguration(m_sAppName, Params);

        //this will make columns in sync log in order they
        //were declared in *.moos file
        Params.reverse();

        STRING_LIST::iterator p;
        for (p = Params.begin(); p != Params.end(); p++) {
            std::string sParam = *p;
            std::string sWhat = MOOSChomp(sParam, "=");

            if (MOOSStrCmp(sWhat, "LOG")) {
                std::string sNewVar = stripBlankEnds(sParam);
                m_vars.push_back(sNewVar);
                m_values[sNewVar] = sqrt(-1.0);
            }
        }

        m_MissionReader.GetConfigurationParam("output_path", m_outputFilePath);

        out.open(m_outputFilePath.c_str());
        printHeader();
    } else {
        std::cout << "Not first time connection." << std::endl;
    }

    RegisterVariables();

    m_started = true;

    return (true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool KST::Iterate() {

    MOOSMSG_LIST list;
    if (m_Comms.Fetch(list)) {
        std::cout << "there is mail" << std::endl;
    } else {
        std::cout << "0" << std::endl;
    }
    printLine();
    return (true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool KST::OnStartUp() {
    return (true);
}

//---------------------------------------------------------
// Procedure: RegisterVariables

void KST::RegisterVariables() {
    for (int i = 0; i < m_vars.size(); i++) {
        m_Comms.Register(m_vars[i], 0);
    }
}

void KST::printHeader() {
    out << "time" << delim;
    for (int i = 0; i < m_vars.size() - 1; i++) {
        out << m_vars[i] << delim;
    }
    out << m_vars.back() << std::endl;
}

void KST::printLine() {
    out << MOOSTime() - m_startTime << delim;
    for (int i = 0; i < m_vars.size(); i++) {
        double val = m_values[m_vars[i]];
        if (isnan(val))
            out << "nan";
        else
            out << val;
        if (i == m_vars.size() - 1)
            out << std::endl;
        else
            out << delim;
        m_values[m_vars[i]] = sqrt(-1.0);
    }
}
