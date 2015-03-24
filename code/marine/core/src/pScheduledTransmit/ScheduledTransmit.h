/*
 * pScheduledTransmit
 *        File: ScheduledTransmit.h
 *  Created on: Apr 9, 2013
 *      Author: Josh Leighton
 */

#ifndef ScheduledTransmit_HEADER
#define ScheduledTransmit_HEADER

#include "MOOS/libMOOS/MOOSLib.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <sstream>

class ScheduledTransmit : public CMOOSApp
{
protected:
    double m_period, m_offset;
    bool m_periodSet, m_offsetSet;

public:
	ScheduledTransmit();
	virtual ~ScheduledTransmit();

	bool OnNewMail(MOOSMSG_LIST &NewMail);
	bool Iterate();
	bool OnConnectToServer();
	bool OnStartUp();

	void setOffset(double offset) {
	    m_offset = offset;
	    m_offsetSet = true;
	}

	void setPeriod(double period) {
	    m_period = period;
	    m_periodSet = true;
	}

protected:
	int m_lastSentSlot;

	bool enabled;

	double getTime();
	int getNextSlot();
	double getSlotTime(int slot);

	void post();
	std::string getRandomString(int length);
};

#endif 
