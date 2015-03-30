/*
 * 
 *        File: SmartCommsVehicle.h
 *  Created on: 
 *      Author: 
 */

#ifndef SmartCommsVehicle_HEADER
#define SmartCommsVehicle_HEADER

#include "MOOS/libMOOS/MOOSLib.h"

class SmartCommsVehicle : public CMOOSApp
{
public:
    SmartCommsVehicle();
    ~SmartCommsVehicle();

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
