/*
 * pAckedCommsVehicle
 *        File: pAckedCommsVehicle.h
 *  Created on: May 26, 2014
 *      Author: Josh Leighton
 */

#ifndef AckedCommsVehicle_HEADER
#define AckedCommsVehicle_HEADER

#include "MOOS/libMOOS/MOOSLib.h"

class AckedCommsVehicle: public CMOOSApp {
public:
    AckedCommsVehicle();
    ~AckedCommsVehicle();

protected:
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();

private:
    // Configuration variables

private:
};

#endif 
