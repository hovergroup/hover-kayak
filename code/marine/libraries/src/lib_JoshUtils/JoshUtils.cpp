/*
 * lib_JoshUtils
 *        File: JoshUtils.h
 *  Created on: Nov 12, 2012
 *      Author: Josh Leighton
 */

#include "JoshUtils.h"

#define MAX_LINE_LENGTH 10000

using namespace std;

void subSearchForFiles(std::vector<std::string> & paths,
        boost::filesystem::path directory_path, int max_depth, std::string wild,
        int current_depth) {
    if (current_depth > max_depth)
        return; // maximum search depth

    // iterate over items in the directory
    boost::filesystem::directory_iterator end_iter;
    for (boost::filesystem::directory_iterator dir_itr(directory_path);
            dir_itr != end_iter; ++dir_itr) {

        try {
            // if file, check file extension
            if (boost::filesystem::is_regular_file(dir_itr->status())) {
#ifdef BOOST_OVER_146
                if (JoshUtil::wildCardMatch(wild,
                        dir_itr->path().filename().string())) {
#else
                    if (JoshUtil::wildCardMatch(wild, dir_itr->path().filename())) {
#endif
                    std::string this_path = boost::filesystem::system_complete(
                            dir_itr->path()).string();
                    if (std::find(paths.begin(), paths.end(), this_path)
                            == (std::vector<std::string>::const_iterator) paths.end())
                        paths.push_back(this_path);
                }
                // if directory, search in that directory after incrementing depth
            } else if (boost::filesystem::is_directory(dir_itr->status()))
                subSearchForFiles(paths,
                        boost::filesystem::system_complete(dir_itr->path()),
                        max_depth, wild, current_depth + 1);
        } catch (const std::exception & ex) {
            std::cout << dir_itr->path().filename() << " " << ex.what()
                    << std::endl;
        }
    }
}

/*
 * This function is a slightly modified version of the one provided
 * with MOOS-IvP.
 */
ALogEntry JoshUtil::getNextRawALogEntry_josh(FILE *fileptr, bool allstrings) {
    ALogEntry entry;
    if (!fileptr) {
        std::cout << "failed getNextRawALogEntry() - null file pointer"
                << std::endl;
        entry.setStatus("invalid");
        return (entry);
    }

    bool EOLine = false;
    bool EOFile = false;
    int buffix = 0;
    int lineix = 0;
    int myint = '\0';
    char buff[MAX_LINE_LENGTH];

    std::string time, var, rawsrc, val;

    // Simple state machine:
    //   0: time
    //   1: between time and variable
    //   2: variable
    //   3: between variable and source
    //   4: source
    //   5: between source and value
    //   6: value
    int state = 0;

    while ((!EOLine) && (!EOFile) && (lineix < MAX_LINE_LENGTH)) {
        //		cout << "state: " << state << endl;
        myint = fgetc(fileptr);
        unsigned char mychar = myint;
        switch (myint) {
        case EOF:
            EOFile = true;
            break;
        case ' ':
            if (state == 6) {
                buff[buffix] = mychar;
                buffix++;
            }
            //			break;
        case '\t':
            if (state == 0) {
                buff[buffix] = '\0';
                time = buff;
                buffix = 0;
                state = 1;
            } else if (state == 2) {
                buff[buffix] = '\0';
                var = buff;
                buffix = 0;
                state = 3;
            } else if (state == 4) {
                buff[buffix] = '\0';
                rawsrc = buff;
                buffix = 0;
                state = 5;
            }
            break;
        case '\n':
            buff[buffix] = '\0'; // attach terminating NULL
            val = buff;
            EOLine = true;
            break;
        default:
            if (state == 1)
                state = 2;
            else if (state == 3)
                state = 4;
            else if (state == 5)
                state = 6;
            buff[buffix] = mychar;
            buffix++;
        }
        lineix++;
    }

    std::string src = biteString(rawsrc, ':');
    std::string srcaux = rawsrc;

    val = stripBlankEnds(val);

    //	cout << "t:" << time << " v:" << var << " s:" << src << " v:" << val << endl;
    if (((time != "") && (var != "") && (src != "") && isNumber(time))
            && (allstrings || !isNumber(val))) {
        entry.set(atof(time.c_str()), var, src, srcaux, val);
    } else if ((time != "") && (var != "") && (src != "") && (val != "")
            && isNumber(time)) {
        entry.set(atof(time.c_str()), var, src, srcaux, atof(val.c_str()));
    } else {
        if (EOFile)
            entry.setStatus("eof");
        else
            entry.setStatus("invalid");
    }

    return (entry);
}

/**
 * returns true if key matches pattern wild ( * for wildcards )
 */
bool JoshUtil::wildCardMatch(std::string wild, std::string key) {
    if (wild == key)
        return true;
    if (wild.empty() || key.empty())
        return false;
    if (wild.find("*") == std::string::npos)
        return false;

    int wild_position = wild.find("*");
    if (wild_position == 0) {
        std::string post_wild = wild.substr(1, wild.size() - 1);
        if (key.size() < post_wild.size())
            return false;
        std::string key_end = key.substr(key.size() - post_wild.size(),
                post_wild.size());
        if (post_wild == key_end)
            return true;
        else
            return false;
    } else if (wild_position == wild.size() - 1) {
        std::string pre_wild = wild.substr(0, wild_position);
        if (key.size() < pre_wild.size())
            return false;
        std::string key_start = key.substr(0, pre_wild.size());
        if (pre_wild == key_start)
            return true;
        else
            return false;
    } else {
        std::string pre_wild = wild.substr(0, wild_position);
        std::string post_wild = wild.substr(wild_position + 1,
                wild.size() - wild_position - 1);
        if (key.size() < pre_wild.size() + post_wild.size())
            return false;
        std::string key_begin = key.substr(0, pre_wild.size());
        std::string key_end = key.substr(key.size() - post_wild.size(),
                post_wild.size());
        if (key_begin == pre_wild && key_end == post_wild)
            return true;
        else
            return false;
    }
}

/**
 * searches directories for files matching wildcard
 * will search in sub folders up to max_depth, starting at 0
 */
void JoshUtil::searchForFiles(std::vector<std::string> & paths,
        std::string directory_path, int max_depth, std::string wild) {
    subSearchForFiles(paths,
            boost::filesystem::system_complete(
                    boost::filesystem::path(directory_path)), max_depth, wild,
            0);
}

double JoshUtil::getSystemTimeSeconds() {
    boost::posix_time::ptime p(
            boost::posix_time::microsec_clock::universal_time());
    return p.time_of_day().total_milliseconds() / 1000.0;
}


/*
 * Test advance to next slot using system time.
 * Returns true if slot is advanced.
 */
bool JoshUtil::TDMAEngine::testAdvance() {
    return testAdvance(getSystemTimeSeconds());
}

/*
 * Test advance to next slot using provided time.
 * Returns true if slot is advanced.
 */
bool JoshUtil::TDMAEngine::testAdvance(double current_time_seconds) {
    // do nothing if stopped or paused
    if (m_run_state == STOPPED) {
        return false;
    }

    int cycle = m_slot_function.getLastSlot(current_time_seconds);
    // check if time within slot is greater than slot length
    if (getTimeWithinSlot(current_time_seconds) > m_slot_lengths[m_current_slot]) {
        // increment slot
        m_current_slot++;

        // if past end, return to first slot
        if (m_current_slot == m_slot_lengths.size()) {
            m_cycle_count++;
            m_current_cycle = cycle;
            m_current_slot = 0;
        }
        return true;
    } else if (cycle != m_current_cycle) {
        m_cycle_count++;
        m_current_cycle = cycle;
        m_current_slot = 0;
        return true;
    } else {
        return false;
    }
}

// m_slotFunctions.getSlotFractionSeconds() > offset

double JoshUtil::TDMAEngine::getTimeWithinSlot() {
    return getTimeWithinSlot(getSystemTimeSeconds());
}

double JoshUtil::TDMAEngine::getTimeWithinSlot(double current_time_seconds) {
    if (getNumSlots() == 0 ) {
        return -1;
    }

    double cycle_position = m_slot_function.getSlotFractionSeconds(current_time_seconds);
    double slot_start = m_slot_times[m_current_slot];
    if (cycle_position < slot_start) {
        return cycle_position + m_slot_function.period - slot_start;
    } else {
        return cycle_position - slot_start;
    }
}

double JoshUtil::TDMAEngine::getTimeToNextSlot() {
    return getTimeToNextSlot(getSystemTimeSeconds());
}

double JoshUtil::TDMAEngine::getTimeToNextSlot(double current_time_seconds) {
    if (getNumSlots() == 0) {
        return -1;
    }

    return m_slot_lengths[m_current_slot] - getTimeWithinSlot(current_time_seconds);
}

bool JoshUtil::TDMAEngine::appendSlot(double length, string name) {
    if (m_run_state != STOPPED) {
        return false;
    }

    m_slot_names.push_back(name);
    if (m_slot_lengths.size() == 0) {
        m_slot_times.push_back(0);
    } else {
        m_slot_times.push_back(m_slot_times.back() + m_slot_lengths.back());
    }
    m_slot_lengths.push_back(length);

    m_slot_function.period = m_slot_times.back() + m_slot_lengths.back();

    return true;
}

bool JoshUtil::TDMAEngine::run() {
    return run(getSystemTimeSeconds());
}

bool JoshUtil::TDMAEngine::run(double current_time_seconds) {
    if (getNumSlots() > 0) {
        m_run_state = RUNNING;
        m_current_slot = 0;
        m_current_cycle = m_slot_function.getLastSlot(current_time_seconds);
        for (int i=0; i<getNumSlots(); i++) {
            if (!testAdvance(current_time_seconds))
                break;
        }
        return true;
    } else {
        return false;
    }
}

void JoshUtil::TDMAEngine::stop() {
    m_run_state = STOPPED;
    m_current_slot = -1;
}

bool JoshUtil::TDMAEngine::parseConfig(CProcessConfigReader & MissionReader, std::string app_name) {
    MissionReader.Reset();
    if (!MissionReader.GoTo("PROCESSCONFIG=" + app_name)) {
        return false;
    }
    bool found_start = false;
    bool found_any = false;
    while (!MissionReader.eof()) {
        string line = MissionReader.GetNextValidLine();
        string sline = line;
        //cout << line << endl;
        MOOSToUpper(line);
        if (line.find("PROCESSCONFIG") != string::npos) {
            break;
        } else if (line == "<TDMA>") {
            found_start = true;
        } else if (line == "</TDMA>") {
            break;
        }
        if (found_start) {
            bool okay = false;
            double length;
            string name = "";
            while (sline.find("=") != string::npos) {
                string byte = MOOSToUpper(MOOSChomp(sline, "="));
                if (byte == "LENGTH") {
                    okay = true;
                    length = atof(MOOSChomp(sline,",").c_str());
                } else if (byte == "NAME") {
                    name = MOOSChomp(sline,",");
                } else {
                    MOOSChomp(sline,",");
                }
            }
            if (okay) {
                found_any = true;
                appendSlot(length, name);
            }
        }
    }
    if (found_any) {
        return true;
    } else {
        return false;
    }
}

string JoshUtil::TDMAEngine::getSummary() {
    char buffer[2048];
    int index = 0;
    index += sprintf(&buffer[index], "%-7s  %-15s\n", "LENGTH", "NAME");
    for (int i=0; i<m_slot_lengths.size(); i++) {
        index += sprintf(&buffer[index], "%-7.2f", m_slot_lengths[i]);
        index += sprintf(&buffer[index], "  ");
        index += sprintf(&buffer[index], "%-15s\n", m_slot_names[i].c_str());
    }
    return string(buffer);
}

string JoshUtil::TDMAEngine::getSingleLineSummary() {
    stringstream ss;
    for (int i=0; i<m_slot_lengths.size(); i++) {
        if (i != 0) {
            ss << ":";
        }
        ss << m_slot_lengths[i] << "," << m_slot_names[i];
    }
    return ss.str();
}
