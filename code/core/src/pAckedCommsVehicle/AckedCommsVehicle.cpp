/*
 * pAckedCommsVehicle
 *        File: pAckedCommsVehicle.cpp
 *  Created on: May 26, 2014
 *      Author: Josh Leighton
 */

#include <iterator>
#include "MBUtils.h"
#include "AckedCommsVehicle.h"
#include "ackedComms.pb.h"

using namespace std;

//---------------------------------------------------------
// Constructor

AckedCommsVehicle::AckedCommsVehicle() {
}

//---------------------------------------------------------
// Destructor

AckedCommsVehicle::~AckedCommsVehicle() {
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool AckedCommsVehicle::OnNewMail(MOOSMSG_LIST &NewMail) {
    MOOSMSG_LIST::iterator p;

    for (p = NewMail.begin(); p != NewMail.end(); p++) {
        CMOOSMsg &msg = *p;
        if (msg.GetKey() == "ACKEDCOMMS_TRANSMIT") {
            AckedTransmissionProto trans_proto;
            if (trans_proto.ParseFromString(msg.GetString())) {
                switch(trans_proto.type()) {
                case AckedTransmissionProto::DOUBLE:
                    m_Comms.Notify(trans_proto.var_name(), trans_proto.double_val());
                    break;
                case AckedTransmissionProto::STRING:
                    m_Comms.Notify(trans_proto.var_name(), trans_proto.string_val());
                    break;
                case AckedTransmissionProto::BINARY_STRING:
                    m_Comms.Notify(trans_proto.var_name(),
                            (void*) trans_proto.string_val().data(),
                            trans_proto.string_val().size());
                    break;
                }
            }
            m_Comms.Notify("ACKEDCOMMS_RETURN_ACK", (double) trans_proto.id());
        }
    }

    return (true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool AckedCommsVehicle::OnConnectToServer() {
    m_Comms.Register("ACKEDCOMMS_TRANSMIT", 0);
    return (true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool AckedCommsVehicle::Iterate() {
    return (true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool AckedCommsVehicle::OnStartUp() {
    return (true);
}
