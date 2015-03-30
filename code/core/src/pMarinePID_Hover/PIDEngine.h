/*
 * pMarinePID_Hover
 *        File: PIDEngine.h
 *  Created on: Aug 20, 2014
 *      Author: Josh Leighton
 */

/*****************************************************************/
/*    NAME: Michael Benjamin, Henrik Schmidt, and John Leonard   */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: PIDEngine.h                                          */
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

#ifndef MARINE_PIDENGINE_HEADER
#define MARINE_PIDENGINE_HEADER

#include <vector>
#include <string>
#include "ScalarPID.h"

enum SpeedControlType {
    FACTOR, PID, FIT_PID
};

class PIDEngine {
public:
    PIDEngine();
    ~PIDEngine() {};

    void setPID(int, ScalarPID);
    void setSpeedFactor(double v) {
        m_speed_factor = v;
    };

    void updateTime(double ctime) {
        m_current_time = ctime;
    };

    double getDesiredRudder(double desired_heading, double current_heading,
            double max_rudder);
    double getDesiredThrust(double desired_speed, double current_speed,
            double current_thrust, double max_thrust,
            double current_heading, double desired_heading);
    double getDesiredElevator(double desired_depth, double current_depth,
            double current_pitch, double max_pitch, double max_elevator);

    void clearReport() {
        m_pid_report.clear();
    };

    std::vector<std::string> getPIDReport() {
        return (m_pid_report);
    };

    void setSpeedControlType(SpeedControlType type) {
        m_speed_control_type = type;
    }

    void setSpeedFunction(double slope, double offset,
            double angle_limit, double time_delay) {
        m_speedSlope = slope;
        m_speedOffset = offset;
        m_angleLimit = angle_limit;
        m_timeDelay = time_delay;
    }

public:
    ScalarPID m_heading_pid;
protected:
    ScalarPID m_speed_pid;
    ScalarPID m_z_to_pitch_pid;
    ScalarPID m_pitch_pid;

    double m_current_time;
    double m_speed_factor;

    std::vector<std::string> m_pid_report;

    // new stuff
    SpeedControlType m_speed_control_type;
    double m_speedSlope, m_speedOffset, m_angleLimit, m_timeDelay;
    double m_headingValidStart, m_currentDelta;
};
#endif

