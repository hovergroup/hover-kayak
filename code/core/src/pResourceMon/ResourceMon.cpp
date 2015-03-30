/*
 * pResourceMon
 *        File: ResourcMon.cpp
 *  Created on: Mar 20, 2014
 *      Author: Josh Leighton
 */

#include <iterator>
#include "MBUtils.h"
#include "ResourceMon.h"
#include <fstream>

using namespace std;

//---------------------------------------------------------
// Constructor

ResourceMon::ResourceMon() {
    m_lastActive = 0;
    m_lastIdle = 0;
}

//---------------------------------------------------------
// Destructor

ResourceMon::~ResourceMon() {
}

bool ResourceMon::Iterate() {
    ifstream fs;

    fs.open("/proc/stat", ifstream::in);
    char input[256];
    fs.getline(input, 256);
    unsigned long user, nice, system, idle, iowait, irq, softirq;
    sscanf(input, "%*s %lu %lu %lu %lu %lu %lu %lu", &user, &nice, &system,
            &idle, &iowait, &irq, &softirq);
    unsigned long total_active = user + nice + system + iowait + irq + softirq;
    double cpu_percent_use = 100 * (total_active - m_lastActive)
            / (total_active - m_lastActive + idle - m_lastIdle);
    m_lastActive = total_active;
    m_lastIdle = idle;
    m_Comms.Notify("CPU_PERCENT_USE", cpu_percent_use);
    fs.close();

    fs.open("/proc/meminfo", ifstream::in);
    unsigned int memtotal, memfree, buffers, cached;
    int complete = 0;
    while (complete < 4) {
        fs.getline(input, 256);
        if (fs.eof())
            break;
        if (sscanf(input, "MemTotal: %u kB", &memtotal) != 0)
            complete++;
        else if (sscanf(input, "MemFree: %u kB", &memfree) != 0)
            complete++;
        else if (sscanf(input, "Buffers: %u kB", &buffers) != 0)
            complete++;
        else if (sscanf(input, "Cached: %u kB", &cached) != 0)
            complete++;
    }
    if (complete == 4) {
        int total_free = memfree + buffers + cached;
        double percent_use = 100 * (memtotal - total_free) / memtotal;
        m_Comms.Notify("MEM_PERCENT_USE", percent_use);
    }
    fs.close();

    m_Comms.Notify("SYSTEM_TIME_SECONDS", JoshUtil::getSystemTimeSeconds());
    return (true);
}

