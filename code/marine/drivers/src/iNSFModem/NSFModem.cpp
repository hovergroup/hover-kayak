/*
 * iNSFModem
 *        File: NSFModem.cpp
 *  Created on: May 7, 2013
 *      Author: Josh Leighton, Pedro Vaz Teixeira
 */

#include <iterator>
#include "MBUtils.h"
#include "NSFModem.h"

using namespace std;

//---------------------------------------------------------
// Constructor

NSFModem::NSFModem() :
        m_power_increase_pin_value("/gpio/boardio5/value"), m_power_increase_pin_direction(
                "/gpio/boardio5/direction"), m_power_decrease_pin_value(
                "/gpio/boardio6/value"), m_power_decrease_pin_direction(
                "/gpio/boardio6/direction"), m_voltage_sense_pin_value(
                "/gpio/boardio7/value"), m_voltage_sense_pin_direction(
                "/gpio/boardio7/direction") {
    m_state = Starting;

    if (!m_power_increase_pin_direction.is_open()) {
        std::cerr << "Unable to open TX power increase pin (direction)\n";
        exit(1);
    }
    if (!m_power_increase_pin_value.is_open()) {
        std::cerr << "Unable to open TX power increase pin (value)\n";
        exit(1);
    }
    if (!m_power_decrease_pin_direction.is_open()) {
        std::cerr << "Unable to open TX power decrease pin (direction)\n";
        exit(1);
    }
    if (!m_power_decrease_pin_value.is_open()) {
        std::cerr << "Unable to open TX power decrease pin (value)\n";
        exit(1);
    }
    if (!m_voltage_sense_pin_direction.is_open()) {
        std::cerr << "Unable to open TX power decrease pin (direction)\n";
        exit(1);
    }
    if (!m_voltage_sense_pin_value.is_open()) {
        std::cerr << "Unable to open Voltage sense pin (value)\n";
    }

    // seems like pins may need to have their state changed to ensure
    // they work correctly

    // configure pins opposite at first
    m_power_increase_pin_direction << "in";
    m_power_increase_pin_direction.flush();
    m_power_decrease_pin_direction << "in";
    m_power_decrease_pin_direction.flush();
    m_voltage_sense_pin_direction << "out";
    m_voltage_sense_pin_direction.flush();

    boost::this_thread::sleep(boost::posix_time::milliseconds(100));

    // reset power level to maximum
    m_power_increase_pin_value << 0;
    m_power_increase_pin_value.flush();
    boost::this_thread::sleep(boost::posix_time::milliseconds(reset_timer)); // sleep
    m_power_increase_pin_value << 1;
    m_power_increase_pin_value.flush();

    // update power level and set requested to match
    m_requested_power_level = 31;
    m_current_power_level = 31;
    print_power_status();

    // start up thread
    m_power_write_thread = boost::thread(
            boost::bind(&NSFModem::power_write_loop, this));

    // set state to running
    m_state = Running;

    // now configure pins to what we actually want
    m_power_increase_pin_direction << "out";
    m_power_increase_pin_direction.flush();
    m_power_decrease_pin_direction << "out";
    m_power_decrease_pin_direction.flush();
    m_voltage_sense_pin_direction << "in";
    m_voltage_sense_pin_direction.flush();
}

//---------------------------------------------------------
// Destructor

NSFModem::~NSFModem() {
    // release pins
    m_power_increase_pin_value.close();
    m_power_decrease_pin_direction.close();
    m_power_decrease_pin_value.close();
    m_power_decrease_pin_direction.close();
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool NSFModem::OnNewMail(MOOSMSG_LIST &NewMail) {
    MOOSMSG_LIST::iterator p;

    for (p = NewMail.begin(); p != NewMail.end(); p++) {
        CMOOSMsg &msg = *p;

        std::string key = msg.GetKey();

        if (key == "NSFMODEM_SET_POWER_LEVEL") {
            powerLevelMutex.lock();
            m_requested_power_level = msg.GetDouble();
            powerLevelMutex.unlock();
            std::cout << "TX power: " << m_requested_power_level << "/"
                    << m_current_power_level << " (requested/current)\n";
        }
    }

    boost::this_thread::sleep(boost::posix_time::milliseconds(30)); // sleep
    return (true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool NSFModem::OnConnectToServer() {
    RegisterVariables();

    // output current state
    print_power_status();

    return (true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool NSFModem::Iterate() {
    char c = m_voltage_sense_pin_value.peek();
    if (c == '1') {
        m_Comms.Notify("NSF_VOLTAGE", "LOW");
    } else if (c == '0') {
        m_Comms.Notify("NSF_VOLTAGE", "OKAY");
    }

    return (true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool NSFModem::OnStartUp() {
    return (true);
}

//---------------------------------------------------------
// Procedure: RegisterVariables

void NSFModem::RegisterVariables() {
    m_Comms.Register("NSFMODEM_SET_POWER_LEVEL", 0);
}

void NSFModem::power_write_loop() {
    while (m_state == Running) {
        // get latest request
        powerLevelMutex.lock();
        int request = m_requested_power_level;

        // reset to max
        if (request > 31) {
            // set desired to max
            m_requested_power_level = 31;
            powerLevelMutex.unlock();

            // hold increase till reset
            m_power_increase_pin_value << 0;
            m_power_increase_pin_value.flush();
            boost::this_thread::sleep(
                    boost::posix_time::milliseconds(reset_timer)); // sleep
            m_power_increase_pin_value << 1;
            m_power_increase_pin_value.flush();

            // update power level
            m_current_power_level = 31;
            print_power_status();
        }

        // reset to min
        else if (request < 0) {
            // set desired to min
            m_requested_power_level = 0;
            powerLevelMutex.unlock();

            // hold decrease till reset
            m_power_decrease_pin_value << 0;
            m_power_decrease_pin_value.flush();
            boost::this_thread::sleep(
                    boost::posix_time::milliseconds(reset_timer)); // sleep
            m_power_decrease_pin_value << 1;
            m_power_decrease_pin_value.flush();

            // update power level
            m_current_power_level = 0;
            print_power_status();
        }

        // increment normally
        else {
            powerLevelMutex.unlock();

            // calculate delta
            int delta = request - m_current_power_level;

            // increase power
            if (delta > 0) {
                std::cout << "Increasing TX power.\n";

                // pulse increase pin once
                m_power_increase_pin_value << 0;
                m_power_increase_pin_value.flush();
                boost::this_thread::sleep(
                        boost::posix_time::milliseconds(debounce_time));
                m_power_increase_pin_value << 1;
                m_power_increase_pin_value.flush();

                // update power level
                m_current_power_level++;
                print_power_status();

                // gap time
                boost::this_thread::sleep(
                        boost::posix_time::milliseconds(gap_time));
            }

            // decrease power
            else if (delta < 0) {
                std::cout << "Decreasing TX power.\n";

                // pulse decrease pin once
                m_power_decrease_pin_value << 0;
                m_power_decrease_pin_value.flush();
                boost::this_thread::sleep(
                        boost::posix_time::milliseconds(debounce_time));
                m_power_decrease_pin_value << 1;
                m_power_decrease_pin_value.flush();

                // update power level
                m_current_power_level--;
                print_power_status();

                // gap time
                boost::this_thread::sleep(
                        boost::posix_time::milliseconds(gap_time));
            }

            // no command
            else {
                boost::this_thread::sleep(boost::posix_time::milliseconds(50));
            }

        }
    }
}

void NSFModem::print_power_status() {
    std::cout << "TX power: current=" << m_current_power_level << ", requested="
            << m_requested_power_level << std::endl;
    m_Comms.Notify("NSFMODEM_CURRENT_POWER_LEVEL", m_current_power_level);
}
