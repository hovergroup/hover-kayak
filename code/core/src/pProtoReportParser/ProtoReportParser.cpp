/*
 * pProtoReportParser
 *        File: ProtoReportParser.cpp
 *  Created on: Sep 11, 2013
 *      Author: Josh Leighton
 */

#include <iterator>
#include "MBUtils.h"
#include "ProtoReportParser.h"
#include "HoverGeometry.h"

using namespace std;

//---------------------------------------------------------
// Constructor

ProtoReportParser::ProtoReportParser() {
}

//---------------------------------------------------------
// Destructor

ProtoReportParser::~ProtoReportParser() {
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool ProtoReportParser::OnNewMail(MOOSMSG_LIST &NewMail) {
    MOOSMSG_LIST::iterator p;

    for (p = NewMail.begin(); p != NewMail.end(); p++) {
        CMOOSMsg &msg = *p;
        std::string key = msg.GetKey();
        if (key == "PROTO_REPORT") {
            ProtoNodeReport pnr;
            if (pnr.ParseFromString(msg.GetString())) {
                if (active_secondaries.find(pnr.vehicle_name()) == active_secondaries.end()) {
                    active_secondaries[pnr.vehicle_name()] = false;
                }

                NodeRecord nr;
                nr.setX(pnr.x());
                nr.setY(pnr.y());
                nr.setLength(2.0);
                nr.setHeading(pnr.heading());
                nr.setSpeed(pnr.speed());
                if (pnr.has_depth())
                    nr.setDepth(pnr.depth());
                nr.setName(pnr.vehicle_name());
                nr.setTimeStamp(pnr.time_stamp());

                switch (pnr.platform_type()) {
                case ProtoNodeReport_PlatformTypeEnum_KAYAK:
                    nr.setType("KAYAK");
                    break;
                case ProtoNodeReport_PlatformTypeEnum_REMUS:
                    nr.setType("AUV");
                    break;
                case ProtoNodeReport_PlatformTypeEnum_GLIDER:
                    nr.setType("GLIDER");
                    break;
                case ProtoNodeReport_PlatformTypeEnum_NSF:
                    nr.setType("GLIDER");
                    break;
                case ProtoNodeReport_PlatformTypeEnum_ICARUS:
                    nr.setType("SHIP");
                    break;
                default:
                    nr.setType("unknown");
                    break;
                }
                m_Comms.Notify("NODE_REPORT", nr.getSpec());

                if (pnr.has_secondary_source()) {
                    NodeRecord nr2;
                    nr2.setHeading(pnr.secondary_heading());
                    nr2.setSpeed(pnr.speed());
                    nr2.setTimeStamp(pnr.time_stamp());
                    nr2.setType(nr.getType());
                    if (pnr.secondary_source() == ProtoNodeReport_SecondarySourceEnum_SECONDARY_INTERNAL) {
                        nr2.setName(pnr.vehicle_name() + "_rtk");
                        nr2.setLength(0.01);
                        nr2.setX(-1000);
                        nr2.setY(1000);
                        m_Comms.Notify("NODE_REPORT", nr2.getSpec());
                        nr2.setLength(1.5);
                        nr2.setX(pnr.secondary_x());
                        nr2.setY(pnr.secondary_y());
                        nr2.setName(pnr.vehicle_name() + "_gps");
                        m_Comms.Notify("NODE_REPORT", nr2.getSpec());
                    } else {
                        nr2.setName(pnr.vehicle_name() + "_gps");
                        nr2.setLength(0.01);
                        nr2.setX(-1000);
                        nr2.setY(1000);
                        m_Comms.Notify("NODE_REPORT", nr2.getSpec());
                        nr2.setLength(1.5);
                        nr2.setX(pnr.secondary_x());
                        nr2.setY(pnr.secondary_y());
                        nr2.setName(pnr.vehicle_name() + "_rtk");
                        m_Comms.Notify("NODE_REPORT", nr2.getSpec());
                    }
                    active_secondaries[pnr.vehicle_name()] = true;
                } else if (active_secondaries[pnr.vehicle_name()] == true) {
                    NodeRecord nr2;
                    nr2.setX(-1000);
                    nr2.setY(1000);
                    nr2.setHeading(pnr.secondary_heading());
                    nr2.setSpeed(pnr.speed());
                    nr2.setTimeStamp(pnr.time_stamp());
                    nr2.setType(nr.getType());

                    nr2.setName(pnr.vehicle_name() + "_rtk");
                    nr2.setLength(0.01);
                    m_Comms.Notify("NODE_REPORT", nr2.getSpec());
                    nr2.setLength(0.01);
                    nr2.setName(pnr.vehicle_name() + "_gps");
                    m_Comms.Notify("NODE_REPORT", nr2.getSpec());

                    active_secondaries[pnr.vehicle_name()] = false;
                }

                for (int i = 0; i < pnr.view_points_size(); i++) {
                    m_Comms.Notify("VIEW_POINT",
                            HoverGeometry::printPoint(pnr.view_points(i)));
                }
                for (int i = 0; i < pnr.view_markers_size(); i++) {
                    m_Comms.Notify("VIEW_MARKER",
                            HoverGeometry::printMarker(pnr.view_markers(i)));
                }
                for (int i = 0; i < pnr.view_polygons_size(); i++) {
                    m_Comms.Notify("VIEW_POLYGON",
                            HoverGeometry::printPolygon(pnr.view_polygons(i)));
                }
                for (int i = 0; i < pnr.view_seglists_size(); i++) {
                    m_Comms.Notify("VIEW_SEGLIST",
                            HoverGeometry::printSeglist(pnr.view_seglists(i)));
                }
            }
        }
    }

    return (true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool ProtoReportParser::OnConnectToServer() {
    // register for variables here
    // possibly look at the mission file?
    // m_MissionReader.GetConfigurationParam("Name", <string>);
    // m_Comms.Register("VARNAME", 0);

    RegisterVariables();
    return (true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool ProtoReportParser::Iterate() {
    return (true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool ProtoReportParser::OnStartUp() {
    return (true);
}

//---------------------------------------------------------
// Procedure: RegisterVariables

void ProtoReportParser::RegisterVariables() {
    m_Comms.Register("PROTO_REPORT");
    // m_Comms.Register("FOOBAR", 0);
}

