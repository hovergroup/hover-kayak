/*
 * pNavManager
 *        File: NavManager.cpp
 *  Created on: Oct 18, 2013
 *      Author: Josh Leighton
 */

#include <iterator>
#include "MBUtils.h"
#include "NavManager.h"

using namespace std;

//---------------------------------------------------------
// Constructor

NavManager::NavManager() {
    source = none;

    TIMEOUT = 5;

    rtk_available = false;
    gps_available = false;
    exp_available = false;
    rtk_point_active = false;
    gps_point_active = false;
    exp_point_active = false;
    last_source_post_time = -1;
    last_alternate_post_time = -1;
}

//---------------------------------------------------------
// Destructor

NavManager::~NavManager() {
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool NavManager::OnNewMail(MOOSMSG_LIST &NewMail) {
    MOOSMSG_LIST::iterator p;

    for (p = NewMail.begin(); p != NewMail.end(); p++) {
        CMOOSMsg &msg = *p;
        string key = msg.GetKey();
        if (key == "GPS_LOCK") {
            if (msg.GetDouble() == 1)
                gps_lock = true;
            else
                gps_lock = false;
        }

        // if using gps, mirror gps, else post point
        else if (key == "GPS_X") {
            gps_x = msg.GetDouble();
            gps_update_time = MOOSTime();
            if (source == gps)
                m_Comms.Notify("NAV_X", gps_x);
        } else if (key == "GPS_Y") {
            gps_y = msg.GetDouble();
            if (source == gps)
                m_Comms.Notify("NAV_Y", gps_y);
        } else if (key == "GPS_SPEED") {
            if (source == gps)
                m_Comms.Notify("NAV_SPEED", msg.GetDouble());
        }

        // if using rtk, mirror rtk
        else if (key == "RTK_X") {
            rtk_x = msg.GetDouble();
            rtk_update_time = MOOSTime();
            if (source == rtk)
                m_Comms.Notify("NAV_X", rtk_x);
        } else if (key == "RTK_Y") {
            rtk_y = msg.GetDouble();
            if (source == rtk)
                m_Comms.Notify("NAV_Y", rtk_y);
        } else if (key == "RTK_SPEED") {
            if (source == rtk)
                m_Comms.Notify("NAV_SPEED", msg.GetDouble());
        }

        // if using experiment, mirror experiment
        else if (key == "EXP_X") {
            exp_x = msg.GetDouble();
            exp_update_time = MOOSTime();
            if (source == exp)
                m_Comms.Notify("NAV_X", exp_x);
        } else if (key == "EXP_Y") {
            exp_y = msg.GetDouble();
            if (source == exp)
                m_Comms.Notify("NAV_Y", exp_y);
        } else if (key == "EXP_SPEED") {
            if (source == exp)
                m_Comms.Notify("NAV_SPEED", msg.GetDouble());
        }

        // use compass unless experiment
        else if (key == "COMPASS_HEADING_FILTERED") {
            compass_heading = msg.GetDouble();
            if (source != exp) {
                m_Comms.Notify("NAV_HEADING", msg.GetDouble());
            }
        } else if (key == "EXP_HEADING") {
            if (source == exp) {
                m_Comms.Notify("NAV_HEADING", msg.GetDouble());
            }
        }

        else if (key == "SET_NAV_SOURCE") {
            if (MOOSToUpper(msg.GetString()) == "EXP") {
                if (exp_available)
                    setSource(exp);
            } else {
                setSource(none);
            }
        }

        else if (key == "RTK_QUALITY") {
            switch ((int) msg.GetDouble()) {
            case 1:
                rtk_status = FIX;
                break;
            case 2:
                rtk_status = FLOAT;
                break;
            case 5:
                rtk_status = SINGLE;
                break;
            default:
                rtk_status = NONE;
                break;
            }
        }
    }

    return (true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool NavManager::OnConnectToServer() {
    m_MissionReader.GetConfigurationParam("timeout", TIMEOUT);
    m_MissionReader.GetValue("Community", my_name);

    bool ok1, ok2, ok3, ok4;
    vector<string> sources(4, "");
    ok1 = m_MissionReader.GetConfigurationParam("source1", sources[0]);
    ok2 = m_MissionReader.GetConfigurationParam("source2", sources[1]);
    ok3 = m_MissionReader.GetConfigurationParam("source3", sources[2]);
    ok4 = m_MissionReader.GetConfigurationParam("source4", sources[3]);

    if (!ok1 || !ok2 || !ok3 || !ok4) {
        cout << "Missing source preference." << endl;
        exit(1);
    }

    source_priorities.clear();
    for (int i = 0; i < sources.size(); i++) {
        MOOSToUpper(sources[i]);
        if (sources[i] == "RTK_FIX")
            source_priorities.push_back(rtk_fix);
        else if (sources[i] == "RTK_FLOAT")
            source_priorities.push_back(rtk_float);
        else if (sources[i] == "RTK_SINGLE")
            source_priorities.push_back(rtk_single);
        else if (sources[i] == "GPS")
            source_priorities.push_back(gps_internal);
        else {
            cout << "Invalid source preference." << endl;
            exit(1);
        }
    }

    rtk_point.set_vx(0);
    rtk_point.set_vy(0);
    rtk_point.set_label(my_name + "_rtk");
    rtk_point.set_vertex_size(3);
    gps_point.set_vx(0);
    gps_point.set_vy(0);
    gps_point.set_label(my_name + "_gps");
    gps_point.set_vertex_size(3);
    exp_point.set_vx(0);
    exp_point.set_vy(0);
    exp_point.set_label(my_name + "_exp");
    exp_point.set_vertex_size(3);

    RegisterVariables();
    return (true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool NavManager::Iterate() {
    // check rtk availability
    if (MOOSTime() - rtk_update_time < TIMEOUT) {
        rtk_available = true;
    } else {
        rtk_available = false;
    }

    // check gps availability
    if (MOOSTime() - gps_update_time < TIMEOUT && gps_lock) {
        gps_available = true;
    } else {
        gps_available = false;
    }

    // check exp availability
    if (MOOSTime() - exp_update_time < TIMEOUT) {
        exp_available = true;
    } else {
        exp_available = false;
    }

    // determine best available source
    NAV_SOURCE best_available = none;
    int i = 0;
    bool decided = false;
    while (i < source_priorities.size() && !decided) {
        switch (source_priorities[i]) {
        case gps_internal:
            if (gps_available && gps_lock) {
                best_available = gps;
                decided = true;
            }
            break;

        case rtk_fix:
            if (rtk_available && rtk_status == FIX) {
                best_available = rtk;
                decided = true;
            }
            break;

        case rtk_float:
            if (rtk_available && rtk_status == FLOAT) {
                best_available = rtk;
                decided = true;
            }
            break;

        case rtk_single:
            if (rtk_available && rtk_status == SINGLE) {
                best_available = none;
                decided = true;
            }
            break;
        }
        i++;
    }

    // if not using experiment, use best available
    if (source != exp) {
        setSource(best_available);
    }

    // if can't use experiment anymore, change to best available
    if (source == exp && !exp_available) {
        setSource(best_available);
    }

    if (MOOSTime() - last_alternate_post_time > 0.2) {
        last_alternate_post_time = MOOSTime();
        // if using experiment, post secondary nav
        if (source == exp && MOOSTime()) {
            switch (best_available) {
            case rtk:
                m_Comms.Notify("SECONDARY_NAV_X", rtk_x);
                m_Comms.Notify("SECONDARY_NAV_Y", rtk_y);
                m_Comms.Notify("SECONDARY_NAV_SOURCE", "rtk");
                break;
            case gps:
                m_Comms.Notify("SECONDARY_NAV_X", gps_x);
                m_Comms.Notify("SECONDARY_NAV_Y", gps_y);
                m_Comms.Notify("SECONDARY_NAV_SOURCE", "gps");
                break;
            default:
                m_Comms.Notify("SECONDARY_NAV_SOURCE", "none");
                break;
            }
            m_Comms.Notify("SECONDARY_NAV_HEADING", compass_heading);

            if (rtk_point.active()) {
                rtk_point.set_active(false);
                m_Comms.Notify("VIEW_POINT", rtk_point.get_spec());
            }
            if (gps_point.active()) {
                gps_point.set_active(false);
                m_Comms.Notify("VIEW_POINT", gps_point.get_spec());
            }
            if (exp_point.active()) {
                exp_point.set_active(false);
                m_Comms.Notify("VIEW_POINT", exp_point.get_spec());
            }
        } else {
            m_Comms.Notify("SECONDARY_NAV_SOURCE", "none");

            // post alternate points
            if (source != rtk && rtk_available) {
                rtk_point.set_vx(rtk_x);
                rtk_point.set_vy(rtk_y);
                rtk_point.set_active(true);
                m_Comms.Notify("VIEW_POINT", rtk_point.get_spec());
                gps_point_active = true;
            } else if (!rtk_available && rtk_point.active()) {
                rtk_point.set_active(false);
                m_Comms.Notify("VIEW_POINT", rtk_point.get_spec());
            }

            if (source != gps && gps_available) {
                gps_point.set_vx(gps_x);
                gps_point.set_vy(gps_y);
                gps_point.set_active(true);
                m_Comms.Notify("VIEW_POINT", gps_point.get_spec());
                gps_point_active = true;
            } else if (!gps_available && gps_point.active()) {
                gps_point.set_active(false);
                m_Comms.Notify("VIEW_POINT", gps_point.get_spec());
            }

            if (source != exp && exp_available) {
                exp_point.set_vx(exp_x);
                exp_point.set_vy(exp_y);
                exp_point.set_active(true);
                m_Comms.Notify("VIEW_POINT", exp_point.get_spec());
                gps_point_active = true;
            } else if (!exp_available && exp_point.active()) {
                exp_point.set_active(false);
                m_Comms.Notify("VIEW_POINT", exp_point.get_spec());
            }
        }
    }



    // post source periodically as a heartbeat
    if (MOOSTime() - last_source_post_time > 5) {
        postSource();
    }

    return (true);
}

void NavManager::postSource() {
    last_source_post_time = MOOSTime();
    switch (source) {
    case gps:
        m_Comms.Notify("NAV_SOURCE", "gps");
        break;
    case rtk:
        m_Comms.Notify("NAV_SOURCE", "rtk");
        break;
    case none:
        m_Comms.Notify("NAV_SOURCE", "none");
        break;
    case exp:
        m_Comms.Notify("NAV_SOURCE", "exp");
        break;
    default:
        break;
    }
}

void NavManager::setSource(NAV_SOURCE new_val) {
    if (new_val != source) {
        source = new_val;
        postSource();
    }
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool NavManager::OnStartUp() {
    return (true);
}

//---------------------------------------------------------
// Procedure: RegisterVariables

void NavManager::RegisterVariables() {
    m_Comms.Register("GPS_X", 0);
    m_Comms.Register("GPS_Y", 0);
    m_Comms.Register("GPS_SPEED", 0);
    m_Comms.Register("GPS_LOCK", 0);
    m_Comms.Register("RTK_X", 0);
    m_Comms.Register("RTK_Y", 0);
    m_Comms.Register("RTK_SPEED", 0);
    m_Comms.Register("RTK_QUALITY", 0);
    m_Comms.Register("EXP_X", 0);
    m_Comms.Register("EXP_Y", 0);
    m_Comms.Register("EXP_SPEED", 0);
    m_Comms.Register("EXP_HEADING", 0);
    m_Comms.Register("COMPASS_HEADING_FILTERED", 0);
    m_Comms.Register("SET_NAV_SOURCE");
}

