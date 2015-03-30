/*
 * 
 *        File: SmartCommsShoreside.h
 *  Created on: 
 *      Author: 
 */

#ifndef SmartCommsShoreside_HEADER
#define SmartCommsShoreside_HEADER

#include "MOOS/libMOOS/MOOSLib.h"

class SmartCommsShoreside : public CMOOSApp
{
public:
    SmartCommsShoreside();
    ~SmartCommsShoreside();

protected:
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();
    void RegisterVariables();

private: // Configuration variables

private: // State variables
    unsigned int m_iterations;
    double       m_timewarp;
};

#endif 
