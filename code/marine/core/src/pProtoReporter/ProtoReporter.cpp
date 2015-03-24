/*
 * pProtoReporter
 *        File: ProtoReporter.cpp
 *  Created on: Sep 11, 2013
 *      Author: Josh Leighton
 */

#include <iterator>
#include "MBUtils.h"
#include "ProtoReporter.h"
#include "HelmReportUtils.h"
#include "HoverGeometry.h"

using namespace std;

//---------------------------------------------------------
// Constructor

ProtoReporter::ProtoReporter() {
    using_rtk = true;
    m_lastNavSourceUpdate = -1;
    m_lastAcommsStatusUpdate = -1;
    m_lastHelmStateUpdate = -1;
    m_lastGPSQualityUpdate = -1;
    m_lastHeadingUpdate = -1;
    m_lastSecondaryUpdate = -1;
}

//---------------------------------------------------------
// Destructor

ProtoReporter::~ProtoReporter() {
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool ProtoReporter::OnNewMail(MOOSMSG_LIST &NewMail) {
    MOOSMSG_LIST::iterator p;

    for (p = NewMail.begin(); p != NewMail.end(); p++) {
        CMOOSMsg &msg = *p;
        std::string key = msg.GetKey();
        if (key == "ACOMMS_DRIVER_STATUS") {
            switch ((int) msg.GetDouble()) {
            case HoverAcomms::READY:
                nr.set_acomms_status(ProtoNodeReport_AcommsStatusEnum_READY);
                break;
            case HoverAcomms::TRANSMITTING:
                nr.set_acomms_status(
                        ProtoNodeReport_AcommsStatusEnum_TRANSMITTING);
                break;
            case HoverAcomms::RECEIVING:
                nr.set_acomms_status(
                        ProtoNodeReport_AcommsStatusEnum_RECEIVING);
                break;
            case HoverAcomms::NOT_RUNNING:
                nr.set_acomms_status(
                        ProtoNodeReport_AcommsStatusEnum_NOT_RUNNING);
                break;
            default:
                break;
            }
            m_lastAcommsStatusUpdate = msg.GetTime();
        } else if (key == "NAV_X") {
            nr.set_x(msg.GetDouble());
        } else if (key == "NAV_Y") {
            nr.set_y(msg.GetDouble());
        } else if (key == "NAV_HEADING") {
            nr.set_heading(msg.GetDouble());
            m_lastHeadingUpdate = msg.GetTime();
        } else if (key == "NAV_SPEED") {
            nr.set_speed(msg.GetDouble());
        } else if (key == "NAV_DEPTH") {
            nr.set_depth(msg.GetDouble());
        } else if (key == "SECONDARY_NAV_X") {
            secondary_x = msg.GetDouble();
        } else if (key == "SECONDARY_NAV_Y") {
            secondary_y = msg.GetDouble();
        } else if (key == "SECONDARY_NAV_HEADING") {
            secondary_heading = msg.GetDouble();
        } else if (key == "SECONDARY_NAV_SOURCE") {
            m_lastSecondaryUpdate = msg.GetTime();
            if (MOOSToUpper(msg.GetString()) == "RTK")
                secondary_source = rtk;
            else if (MOOSToUpper(msg.GetString()) == "GPS")
                secondary_source = gps;
            else
                secondary_source = none;
        } else if (key == "VOLTAGE") {
            nr.set_voltage(msg.GetDouble());
        } else if (key == "NSF_VOLTAGE") {
            if (MOOSToUpper(msg.GetString()) == "OKAY") {
                nr.set_nsf_power(ProtoNodeReport_NSFPowerEnum_OKAY);
            } else {
                nr.set_nsf_power(ProtoNodeReport_NSFPowerEnum_LOW);
            }
        } else if (key == "IVPHELM_STATE") {
            if (msg.GetString() == "DRIVE") {
                nr.set_helm_state(ProtoNodeReport_HelmStateEnum_DRIVE);
            } else if (msg.GetString() == "MALCONFIG") {
                nr.set_helm_state(ProtoNodeReport_HelmStateEnum_MALCONFIG);
            } else if (msg.GetString() == "PARK") {
                nr.set_helm_state(ProtoNodeReport_HelmStateEnum_PARK);
            } else {
                nr.set_helm_state(ProtoNodeReport_HelmStateEnum_UNKNOWN);
            }
            m_lastHelmStateUpdate = msg.GetTime();
        } else if (key == "RTK_QUALITY" && using_rtk) {
            switch ((int) msg.GetDouble()) {
            case 1:
                nr.set_gps_quality(ProtoNodeReport_GPSQualityEnum_FIX);
                break;
            case 2:
                nr.set_gps_quality(ProtoNodeReport_GPSQualityEnum_FLOAT);
                break;
            case 5:
                nr.set_gps_quality(ProtoNodeReport_GPSQualityEnum_SINGLE);
                break;
            default:
                break;
            }
            m_lastGPSQualityUpdate = msg.GetTime();
        } else if (key == "IVPHELM_SUMMARY") {
            vector<string> svector = parseStringQ(msg.GetString(), ',');
            unsigned int i, vsize = svector.size();
            for (i = 0; i < vsize; i++) {
                string left = biteStringX(svector[i], '=');
                string right = svector[i];

                if (left == "active_bhvs") {
                    nr.clear_active_behaviors();
                    nr.add_active_behaviors(biteStringX(right, '$'));

                    while (right.find(":") != string::npos) {
                        biteStringX(right, ':');
                        nr.add_active_behaviors(biteStringX(right, '$'));
                    }
                }
            }

            //"iter=45,utc_time=1379176756.94,ofnum=2,var=speed:2,var=course:163,
            //active_bhvs=goto_and_station$1379176756.94$100.00000$9$0.01000$1/1$1:
            //goto_and_return$1379176756.94$100.00000$9$0.01000$1/1$1,
            //idle_bhvs=return$0.00$n/a:Archie_Stationkeep$1379176756.94$n/a"
        } else if (key == "RADIO_POWER") {
            string val = MOOSToUpper(msg.GetString());
            if (val == "BULLET_UNLOCKED") {
                nr.set_radio_state(
                        ProtoNodeReport_RadioStateEnum_BULLET_UNLOCKED);
            } else if (val == "BULLET_LOCKED") {
                nr.set_radio_state(
                        ProtoNodeReport_RadioStateEnum_BULLET_LOCKED);
            } else if (val == "FREEWAVE_UNLOCKED") {
                nr.set_radio_state(
                        ProtoNodeReport_RadioStateEnum_FREEWAVE_UNLOCKED);
            } else if (val == "FREEWAVE_LOCKED") {
                nr.set_radio_state(
                        ProtoNodeReport_RadioStateEnum_FREEWAVE_LOCKED);
            }
        } else if (key == "NAV_SOURCE") {
            m_lastNavSourceUpdate = msg.GetTime();
            string val = MOOSToUpper(msg.GetString());
            if (val == "RTK") {
                using_rtk = true;
            } else if (val == "GPS") {
                using_rtk = false;
                nr.set_gps_quality(ProtoNodeReport_GPSQualityEnum_INTERNAL);
            } else if (val == "NONE") {
                using_rtk = false;
                nr.set_gps_quality(ProtoNodeReport_GPSQualityEnum_NO_GPS);
            } else if (val == "EXP") {
                using_rtk = false;
                nr.set_gps_quality(ProtoNodeReport_GPSQualityEnum_EXPERIMENT);
            } else {
                using_rtk = false;
                nr.set_gps_quality(ProtoNodeReport_GPSQualityEnum_NO_MANAGER);
            }
        } else if (key == "NSFMODEM_CURRENT_POWER_LEVEL") {
            nr.set_nsf_power_level((int) msg.GetDouble());
        } else if (key == "THRUST_LIMIT") {
            nr.set_thrust_limit((int) msg.GetDouble());
        } else if (key == "ARDUINO_THRUST") {
            nr.set_thrust((int) msg.GetDouble());
        } else if (key == "CPU_PERCENT_USE") {
            nr.set_cpu_percent_use((int) msg.GetDouble());
        } else if (key == "MEM_PERCENT_USE") {
            nr.set_mem_percent_use((int) msg.GetDouble());
        } else if (key == "VIEW_POINT") {
            // parse the message
            VIEW_POINT point;
            if (HoverGeometry::parsePoint(msg.GetString(), point)) {
                // check if we already have a matching label
                VIEW_POINT * p;
                bool exists = false;
                for (int i = 0; i < nr.view_points_size(); i++) {
                    // if we do, replace that one
                    if (nr.view_points(i).label() == point.label()) {
                        p = nr.mutable_view_points(i);
                        exists = true;
                        break;
                    }
                }
                // if we don't, add a new entry
                if (!exists)
                    p = nr.add_view_points();
                p->CopyFrom(point);
            }
        } else if (key == "VIEW_MARKER") {
            // parse the message
            VIEW_MARKER marker;
            if (HoverGeometry::parseMarker(msg.GetString(), marker)) {
                // check if we already have a matching label
                VIEW_MARKER * p;
                bool exists = false;
                for (int i = 0; i < nr.view_markers_size(); i++) {
                    // if we do, replace that one
                    if (nr.view_markers(i).label() == marker.label()) {
                        p = nr.mutable_view_markers(i);
                        exists = true;
                        break;
                    }
                }
                // if we don't, add a new entry
                if (!exists)
                    p = nr.add_view_markers();
                p->CopyFrom(marker);
            }
        } else if (key == "VIEW_SEGLIST") {
            // parse the message
            VIEW_SEGLIST seglist;
            if (HoverGeometry::parseSeglist(msg.GetString(), seglist)) {
                // check if we already have a matching label
                VIEW_SEGLIST * p;
                bool exists = false;
                for (int i = 0; i < nr.view_seglists_size(); i++) {
                    // if we do, replace that one
                    if (nr.view_seglists(i).label() == seglist.label()) {
                        p = nr.mutable_view_seglists(i);
                        exists = true;
                        break;
                    }
                }
                // if we don't, add a new entry
                if (!exists)
                    p = nr.add_view_seglists();
                p->CopyFrom(seglist);
            }
        } else if (key == "VIEW_POLYGON") {
            // parse the message
            VIEW_POLYGON polygon;
            if (HoverGeometry::parsePolygon(msg.GetString(), polygon)) {
                // check if we already have a matching label
                VIEW_POLYGON * p;
                bool exists = false;
                for (int i = 0; i < nr.view_polygons_size(); i++) {
                    // if we do, replace that one
                    if (nr.view_polygons(i).label() == polygon.label()) {
                        p = nr.mutable_view_polygons(i);
                        exists = true;
                        break;
                    }
                }
                // if we don't, add a new entry
                if (!exists)
                    p = nr.add_view_polygons();
                p->CopyFrom(polygon);
            }
        }
    }

    return (true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool ProtoReporter::OnConnectToServer() {
    // register for variables here
    // possibly look at the mission file?
    // m_MissionReader.GetConfigurationParam("Name", <string>);
    // m_Comms.Register("VARNAME", 0);
    m_MissionReader.GetValue("Community", m_name);
    nr.set_vehicle_name(m_name);

    std::string platform;
    m_MissionReader.GetConfigurationParam("PLATFORM_TYPE", platform);
    platform = MOOSToUpper(platform.c_str());
    if (platform == "KAYAK") {
        nr.set_platform_type(ProtoNodeReport_PlatformTypeEnum_KAYAK);
    } else if (platform == "GLIDER") {
        nr.set_platform_type(ProtoNodeReport_PlatformTypeEnum_GLIDER);
    } else if (platform == "REMUS") {
        nr.set_platform_type(ProtoNodeReport_PlatformTypeEnum_REMUS);
    } else if (platform == "NSF") {
        nr.set_platform_type(ProtoNodeReport_PlatformTypeEnum_NSF);
    } else if (platform == "ICARUS") {
        nr.set_platform_type(ProtoNodeReport_PlatformTypeEnum_ICARUS);
    }

    RegisterVariables();

    m_Comms.Notify("IVPHELM_REJOURNAL", "true");
    return (true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool ProtoReporter::Iterate() {
    if (MOOSTime() - m_lastAcommsStatusUpdate > 6) {
        nr.set_acomms_status(ProtoNodeReport_AcommsStatusEnum_OFFLINE);
    }
    if (MOOSTime() - m_lastHelmStateUpdate > 5) {
        nr.set_helm_state(ProtoNodeReport_HelmStateEnum_MISSING);
    }
    if (MOOSTime() - m_lastGPSQualityUpdate > 5 && using_rtk) {
        nr.set_gps_quality(ProtoNodeReport_GPSQualityEnum_NO_GPS);
    }
    if (MOOSTime() - m_lastNavSourceUpdate > 6) {
        nr.set_gps_quality(ProtoNodeReport_GPSQualityEnum_NO_MANAGER);
    }
    if (MOOSTime() - m_lastHeadingUpdate > 5
            && nr.platform_type() == ProtoNodeReport_PlatformTypeEnum_KAYAK) {
        nr.add_errors(ProtoNodeReport_ErrorEnum_NoCompassData);
    }
    if (MOOSTime() - m_lastSecondaryUpdate > 5) {
        secondary_source = none;
    }

    nr.set_time_stamp(MOOSTime());

    if (secondary_source == none) {
        nr.clear_secondary_x();
        nr.clear_secondary_y();
        nr.clear_secondary_heading();
        nr.clear_secondary_source();
    } else {
        switch (secondary_source) {
        case gps:
            nr.set_secondary_source(ProtoNodeReport_SecondarySourceEnum_SECONDARY_INTERNAL);
            break;
        case rtk:
            nr.set_secondary_source(ProtoNodeReport_SecondarySourceEnum_SECONDARY_RTK);
            break;
        }
        nr.set_secondary_x(secondary_x);
        nr.set_secondary_y(secondary_y);
        nr.set_secondary_heading(secondary_heading);
    }

    cout << nr.DebugString() << endl;

    std::string out = nr.SerializeAsString();
    cout << "size = " << out.size() << endl << endl << endl;
    if (!out.empty())
        m_Comms.Notify("PROTO_REPORT_LOCAL", (void*) out.data(), out.size());

    // clear view objects every time we send
    nr.clear_view_points();
    nr.clear_view_markers();
    nr.clear_view_seglists();
    nr.clear_view_polygons();
    nr.clear_errors();

    return (true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool ProtoReporter::OnStartUp() {
    return (true);
}

//---------------------------------------------------------
// Procedure: RegisterVariables

void ProtoReporter::RegisterVariables() {
    m_Comms.Register("NAV_X", 0);
    m_Comms.Register("NAV_Y", 0);
    m_Comms.Register("NAV_HEADING", 0);
    m_Comms.Register("NAV_SPEED", 0);
    m_Comms.Register("NAV_DEPTH", 0);
    m_Comms.Register("SECONDARY_NAV_X", 0);
    m_Comms.Register("SECONDARY_NAV_Y", 0);
    m_Comms.Register("SECONDARY_NAV_HEADING", 0);
    m_Comms.Register("SECONDARY_NAV_SOURCE", 0);
    m_Comms.Register("ACOMMS_DRIVER_STATUS", 0);
    m_Comms.Register("IVPHELM_STATE", 0);
    m_Comms.Register("IVPHELM_SUMMARY", 0);
    m_Comms.Register("RTK_QUALITY", 0);
    m_Comms.Register("NAV_SOURCE", 0);
    m_Comms.Register("CPU_PERCENT_USE", 0);
    m_Comms.Register("MEM_PERCENT_USE", 0);
    m_Comms.Register("VIEW_POINT", 0);
    m_Comms.Register("VIEW_SEGLIST", 0);
    m_Comms.Register("VIEW_MARKER", 0);
    m_Comms.Register("VIEW_POLYGON", 0);
    switch (nr.platform_type()) {
    case ProtoNodeReport_PlatformTypeEnum_NSF:
        m_Comms.Register("NSF_VOLTAGE", 0);
        m_Comms.Register("NSFMODEM_CURRENT_POWER_LEVEL", 0);
        break;
    case ProtoNodeReport_PlatformTypeEnum_KAYAK:
        m_Comms.Register("VOLTAGE", 0);
        m_Comms.Register("THRUST_LIMIT", 0);
        m_Comms.Register("ARDUINO_THRUST", 0);
        m_Comms.Register("RADIO_POWER", 0);
        break;
    case ProtoNodeReport_PlatformTypeEnum_ICARUS:
        m_Comms.Register("VOLTAGE", 0);
        break;
    }
}

