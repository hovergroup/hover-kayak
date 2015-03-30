/*
 * pMarinePID_Hover
 *        File: PIDEngine.cpp
 *  Created on: Aug 20, 2014
 *      Author: Josh Leighton
 */

/*****************************************************************/
/*    NAME: Michael Benjamin, Henrik Schmidt, and John Leonard   */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: PIDEngine.cpp                                        */
/*    DATE: Jul 31st, 2005 Sunday in Montreal                    */
/*                                                               */
/* This program is free software; you can redistribute it and/or */
/* modify it under the terms of the GNU General Public License   */
/* as published by the Free Software Foundation; either version  */
/* 2 of the License, or (at your option) any later version.      */
/*                                                               */
/* This program is distributed in the hope that it will be       */
/* useful, but WITHOUT ANY WARRANTY; without even the implied    */
/* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR       */
/* PURPOSE. See the GNU General Public License for more details. */
/*                                                               */
/* You should have received a copy of the GNU General Public     */
/* License along with this program; if not, write to the Free    */
/* Software Foundation, Inc., 59 Temple Place - Suite 330,       */
/* Boston, MA 02111-1307, USA.                                   */
/*****************************************************************/

#ifdef _WIN32
#pragma warning(disable : 4786)
#endif

#include <iostream>
#include "PIDEngine.h"
#include "MBUtils.h"
#include "AngleUtils.h"
#include "MOOS/libMOOS/MOOSLib.h"
#include <math.h>

using namespace std;

//-----------------------------------------------------------
// Procedure: Constructor

PIDEngine::PIDEngine() {
    // If spid_active is zero speed is controlled via PID.
    // If not, thrust is set to multiple of desired speed.
    m_speed_factor = 20.0;
    m_current_time = 0;

    m_headingValidStart = -1;
    m_currentDelta = 0;
}

//------------------------------------------------------------
// Procedure: getDesiredRudder
// Rudder angles are processed in degrees

double PIDEngine::getDesiredRudder(double desired_heading,
        double current_heading, double max_rudder) {
    desired_heading = angle180(desired_heading);
    double heading_error = current_heading - desired_heading;
    heading_error = angle180(heading_error);
    double desired_rudder = 0;
    m_heading_pid.Run(current_heading, desired_heading, m_current_time,
            desired_rudder, true);
    desired_rudder *= -1.0;

    // Enforce limit on desired rudder
    MOOSAbsLimit(desired_rudder, max_rudder);

    string rpt = "PID_COURSE: ";
    rpt += " (Want):" + doubleToString(desired_heading);
    rpt += " (Curr):" + doubleToString(current_heading);
    rpt += " (Diff):" + doubleToString(heading_error);
    rpt += " RUDDER:" + doubleToString(desired_rudder);
    m_pid_report.push_back(rpt);
    return (desired_rudder);
}

//------------------------------------------------------------
// Procedure: getDesiredThrust

double PIDEngine::getDesiredThrust(double desired_speed, double current_speed,
        double current_thrust, double max_thrust,
        double current_heading, double desired_heading) {

    double speed_error = desired_speed - current_speed;
    double delta_thrust = 0;
    double desired_thrust = current_thrust;

    switch (m_speed_control_type) {
    case FACTOR: // unchanged from original
    {
        desired_thrust = desired_speed * m_speed_factor;

        if (desired_thrust < 0)
            desired_thrust = 0;

        string rpt = "PID_SPEED: ";
        rpt += " (Want):" + doubleToString(desired_speed);
        rpt += " (Curr):" + doubleToString(current_speed);
        rpt += " (Diff):" + doubleToString(speed_error);
        rpt += " (Fctr):" + doubleToString(m_speed_factor);
        rpt += " THRUST:" + doubleToString(desired_thrust);
        m_pid_report.push_back(rpt);

        break;
    }

    case PID: // unchanged from original
    {
        m_speed_pid.Run(current_speed, desired_speed, m_current_time,
                delta_thrust);
        desired_thrust += delta_thrust;

        if (desired_thrust < 0)
            desired_thrust = 0;

        string rpt = "PID_SPEED: ";
        rpt += " (Want):" + doubleToString(desired_speed);
        rpt += " (Curr):" + doubleToString(current_speed);
        rpt += " (Diff):" + doubleToString(speed_error);
        rpt += " (Delt):" + doubleToString(delta_thrust);
        rpt += " THRUST:" + doubleToString(desired_thrust);
        m_pid_report.push_back(rpt);

        break;
    }

    case FIT_PID:
    {
        desired_thrust = desired_speed*m_speedSlope + m_speedOffset;
//        cout << "Base thrust = "  << desired_thrust << endl;

        double anglediff = current_heading-desired_heading;
        while (anglediff < -180) anglediff += 180;
        while (anglediff > 180) anglediff -= 180;

        if (fabs(anglediff) < m_angleLimit) {
//            cout << "Within angle limit" << endl;
            // heading is valid
            if (m_headingValidStart == -1) {
//                cout << "Starting timer" << endl;
                // first time valid
                m_headingValidStart = m_current_time;
                desired_thrust += m_currentDelta;
            } else if (m_current_time - m_timeDelay > m_headingValidStart) {
//                cout << "Running PID" << endl;
                // valid for long enough
                m_speed_pid.Run(current_speed, desired_speed, m_current_time,
                        delta_thrust);
                m_currentDelta = delta_thrust;
                desired_thrust += delta_thrust;
            } else {
//                cout << "Waiting on timer" << endl;
                // has not been valid for long enough yet
                desired_thrust += m_currentDelta;
            }
        } else {
//            cout << "Outside angle limit" << endl;
            // heading is not valid
            m_headingValidStart = -1;
            desired_thrust += m_currentDelta;
        }

//        cout << "Output " << desired_thrust << endl;

        if (desired_thrust < 0)
            desired_thrust = 0;

        string rpt = "PID_SPEED: ";
        rpt += " (Want):" + doubleToString(desired_speed);
        rpt += " (Curr):" + doubleToString(current_speed);
        rpt += " (Diff):" + doubleToString(speed_error);
        rpt += " (Delt):" + doubleToString(m_currentDelta);
        rpt += " THRUST:" + doubleToString(desired_thrust);
        m_pid_report.push_back(rpt);

        break;
    }

    }

    // Enforce limit on desired thrust
    MOOSAbsLimit(desired_thrust, max_thrust);

    return (desired_thrust);
}

//------------------------------------------------------------
// Procedure: getDesiredElevator
// Elevator angles and pitch are processed in radians

double PIDEngine::getDesiredElevator(double desired_depth, double current_depth,
        double current_pitch, double max_pitch, double max_elevator) {
    double desired_elevator = 0;
    double desired_pitch = 0;
    double depth_error = current_depth - desired_depth;
    m_z_to_pitch_pid.Run(current_depth, desired_depth, m_current_time,
            desired_pitch);

    // Enforce limits on desired pitch
    MOOSAbsLimit(desired_pitch, max_pitch);

    double pitch_error = current_pitch - desired_pitch;
    m_pitch_pid.Run(current_pitch, desired_pitch, m_current_time,
            desired_elevator);

    // Convert desired elevator to degrees
    desired_elevator = MOOSRad2Deg(desired_elevator);

    // Enforce elevator limit
    MOOSAbsLimit(desired_elevator, max_elevator);

    string rpt = "PID_DEPTH: ";
    rpt += " (Want):" + doubleToString(desired_depth);
    rpt += " (Curr):" + doubleToString(current_depth);
    rpt += " (Diff):" + doubleToString(depth_error);
    rpt += " ELEVATOR:" + doubleToString(desired_elevator);
    m_pid_report.push_back(rpt);

    return (desired_elevator);
}

//-----------------------------------------------------------
// Procedure: setPID

void PIDEngine::setPID(int ix, ScalarPID g_pid) {
    if (ix == 0)
        m_heading_pid = g_pid;
    else if (ix == 1)
        m_speed_pid = g_pid;
    else if (ix == 2)
        m_z_to_pitch_pid = g_pid;
    else if (ix == 3)
        m_pitch_pid = g_pid;
}

