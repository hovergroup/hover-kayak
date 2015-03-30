/*
 * pNavManager
 *        File: NavManager.h
 *  Created on: Oct 18, 2013
 *      Author: Josh Leighton
 */

#ifndef NavManager_HEADER
#define NavManager_HEADER

#include "MOOS/libMOOS/MOOSLib.h"
#include "XYPoint.h"
#include <vector>

enum NAV_SOURCE {
    gps = 0, rtk, none, exp
};

class NavManager: public CMOOSApp {
public:
    NavManager();
    ~NavManager();

protected:
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();
    void RegisterVariables();

private:
    double TIMEOUT;

private:
    NAV_SOURCE source;
    bool gps_lock, rtk_available, gps_available, exp_available;
    double gps_update_time, rtk_update_time, exp_update_time;

    double rtk_x, rtk_y;
    double gps_x, gps_y;
    double exp_x, exp_y;
    double compass_heading;

    double last_alternate_post_time;
    bool rtk_point_active, gps_point_active, exp_point_active;
    XYPoint rtk_point, gps_point, exp_point;

    double last_source_post_time;

    std::string my_name;

    void setSource(NAV_SOURCE new_val);
    void postSource();

    enum RTK_STATUS {
        FIX, FLOAT, SINGLE, NONE
    };
    RTK_STATUS rtk_status;

    enum DETAILED_SOURCE {
        gps_internal, rtk_fix, rtk_float, rtk_single
    };
    std::vector<DETAILED_SOURCE> source_priorities;
};

#endif
