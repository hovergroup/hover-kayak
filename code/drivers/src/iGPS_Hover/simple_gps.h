/*
 * iGPS_Hover
 *        File: simple_gps.h
 *  Created on: Jul 24, 2012
 *      Author: Josh Leighton
 */

#ifndef __SERIAL_H
#define __SERIAL_H

#include "MOOS/libMOOS/MOOSLib.h"
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <fstream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "MOOS/libMOOSGeodesy/MOOSGeodesy.h"
#include "MOOS/libMOOS/App/MOOSInstrument.h"

class SIMPLE_GPS: public CMOOSInstrument {
public:
    SIMPLE_GPS();
    virtual ~SIMPLE_GPS() {};

    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();
    void RegisterVariables();

    void open_port(std::string port_name, int baudRate);
    void close_port();

    void writeData(unsigned char *ptr, int length);

private:
    std::ofstream m_gps_log;
    bool driver_initialized;

    CMOOSGeodesy m_Geodesy;

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

    std::vector<unsigned char> readBuffer, writeBuffer;
    boost::mutex writeBufferMutex;
    int bytesToWrite;

    // parsing operations
    void parseGPRMC(std::string sNMEAString);
    void parseGPGGA(std::string sNMEAString);
    void parseGPGST(std::string sNMEAString);
    void parseLine(std::string msg);

    // the background loop responsible for interacting with the serial port
    void serialLoop();
    void processWriteBuffer();
    void processReadBuffer();

    bool file_exists(std::string filename);
    void writeLine(std::string sLine);

    int m_tcp_sockfd;
    bool m_use_tcp;
    std::vector<char> tcpReadBuffer;

};

#endif
