/*
 * lib_JoshUtils
 *        File: JoshUtils.cpp
 *  Created on: Nov 12, 2012
 *      Author: Josh Leighton
 */

#ifndef LIB_JOSHUTIL_H_
#define LIB_JOSHUTIL_H_

#include "MOOS/libMOOS/Utils/ProcessConfigReader.h"
#include "LogUtils.h"
#include "MBUtils.h"
#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <boost/filesystem.hpp>
#include <exception>
#include <algorithm>
#include <sstream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "MOOS/libMOOS/Utils/MOOSUtilityFunctions.h"

#include "config.h"

namespace JoshUtil {

ALogEntry getNextRawALogEntry_josh(FILE *fileptr, bool allstrings = false);
bool wildCardMatch(std::string wild, std::string key);

void searchForFiles(std::vector<std::string> & paths,
        std::string directory_path, int max_depth, std::string wild);

double getSystemTimeSeconds();

// Slot functions:
class SlotFunctions {
public:
    SlotFunctions() {
        base_offset = 0;
    }

    double period, base_offset;

    // get the previous/current slot
    int getLastSlot(double time) {
        return floor((time - base_offset) / period);
    }
    int getLastSlot() {
        return getLastSlot(getSystemTimeSeconds());
    }

    // get fractional position inside a slot
    double getSlotFraction(double time) {
        double d;
        return modf((time - base_offset) / period, &d);
    }
    double getSlotFraction() {
        return getSlotFraction(getSystemTimeSeconds());
    }

    // get seconds into a slot
    double getSlotFractionSeconds(double time) {
        return getSlotFraction(time) * period;
    }
    double getSlotFractionSeconds() {
        return getSlotFractionSeconds(getSystemTimeSeconds());
    }

    // get the next slot
    int getNextSlot(double time) {
        return ceil((time - base_offset) / period);
    }
    int getNextSlot() {
        return getNextSlot(getSystemTimeSeconds());
    }

    // convert slot to system time
    double slot2seconds(int slot) {
        return slot * period + base_offset;
    }
};

class TDMAEngine {
public:
    enum RunState {
        STOPPED, RUNNING
    };

    TDMAEngine() {
        m_base_offset = 0;
        m_run_state = STOPPED;
        m_current_slot = -1;
        m_cycle_count = 0;
        m_current_cycle = 0;
    }

    bool testAdvance(double current_time_seconds);
    bool testAdvance();

    int getCurrentSlot() { return m_current_slot; }
    std::string getCurrentSlotName() { return m_slot_names[m_current_slot]; }
    int getCycleCount() { return m_cycle_count; }
    int getCycleNumber() { return m_current_cycle; }

    double getTimeToNextSlot(double current_time_seconds);
    double getTimeWithinSlot(double current_time_seconds);
    double getTimeToNextSlot();
    double getTimeWithinSlot();

    int getNumSlots() { return m_slot_lengths.size(); }
    std::vector<double> getSlotLengths() { return m_slot_lengths; }
    std::vector<std::string> getSlotNames() { return m_slot_names; }

    bool appendSlot(double length, std::string name = "");
    bool parseConfig(CProcessConfigReader & MissionReader, std::string app_name);
    std::string getSummary();
    std::string getSingleLineSummary();

    bool run();
    bool run(double current_time_seconds);
    void stop();
    void reset() { m_cycle_count = 0; }
    RunState getRunState () { return m_run_state; }

protected:

    double m_base_offset;
    std::vector<double> m_slot_lengths, m_slot_times;
    std::vector<std::string> m_slot_names;
    int m_current_slot, m_cycle_count, m_current_cycle;

    RunState m_run_state;
    SlotFunctions m_slot_function;
};

}; // namespace JoshUtil

#endif /* LIB_JOSHUTIL_H_ */
