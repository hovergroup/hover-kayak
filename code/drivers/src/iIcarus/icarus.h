/*
 * iIcarus
 *        File: icarus.h
 *  Created on: Mar 11, 2014
 *      Author: Josh Leighton
 */

#ifndef __SERIAL_H
#define __SERIAL_H

#include "MOOS/libMOOS/MOOSLib.h"
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <math.h>
#include <stdio.h>

class icarus_driver: public CMOOSApp {
public:
    icarus_driver();
    virtual ~icarus_driver() {
    }
    ;

    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();
    void RegisterVariables();

    void open_port(std::string port_name, int baudRate);
    void close_port();

private:
    // basic serial port components
    boost::asio::io_service io;
    boost::asio::serial_port port;

    boost::thread serial_thread;
    bool stop_requested;

    std::string my_port_name;
    int my_baud_rate;

    // for asyncronous serial port operations
    boost::asio::deadline_timer timeout;
    bool data_available;
    // ----
    void read_handler(bool& data_available,
            boost::asio::deadline_timer& timeout,
            const boost::system::error_code& error,
            std::size_t bytes_transferred);
    void wait_callback(boost::asio::serial_port& ser_port,
            const boost::system::error_code& error);
    void null_handler(const boost::system::error_code& error,
            std::size_t bytes_transferred) {};

    std::vector<char> readBuffer;
    int buffer_index;

    // the background loop responsible for interacting with the serial port
    void serialLoop();

    void shiftBuffer(int shift);
    int processBuffer();
    int findLine(int index);

    void parseIcarus(int index, int stopIndex);
};

#endif
