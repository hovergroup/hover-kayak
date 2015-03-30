/*
 * iAltimeter
 *        File: iAltimeter.h
 *  Created on: Aug 3, 2012
 *      Author: Josh Leighton
 */

#ifndef __SERIAL_H
#define __SERIAL_H

#include "MOOS/libMOOS/MOOSLib.h"
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

class ALTIMETER: public CMOOSApp {
public:
    ALTIMETER();
    virtual ~ALTIMETER() {};

    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();
    void RegisterVariables();

    void open_port(std::string port_name, int baudRate);
    void close_port();

    void writeData(unsigned char *ptr, int length);

private:
    // basic serial port components
    boost::asio::io_service io;
    boost::asio::serial_port port;

    boost::thread serial_thread;
    bool stop_requested;

    std::string my_port_name;
    int my_baud_rate;

    std::string string_buffer;

    // for asyncronous serial port operations
    boost::asio::deadline_timer timeout;
    bool data_available;
    int asyncBytesRead;
    // ----
    void read_handler(bool& data_available,
            boost::asio::deadline_timer& timeout,
            const boost::system::error_code& error,
            std::size_t bytes_transferred);
    void wait_callback(boost::asio::serial_port& ser_port,
            const boost::system::error_code& error);
    void null_handler(const boost::system::error_code& error,
            std::size_t bytes_transferred) {
    }
    ;

    std::vector<unsigned char> readBuffer, writeBuffer;
    boost::mutex writeBufferMutex;
    int bytesToWrite;

    void parseLine(std::string msg);

    // the background loop responsible for interacting with the serial port
    void serialLoop();
    void processWriteBuffer();
};

#endif
