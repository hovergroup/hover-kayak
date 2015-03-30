/*
 * iRTKNAVI
 *        File: RTKNAVI.h
 *  Created on: May 31, 2013
 *      Author: Josh Leighton
 */

#ifndef RTKNAVI_HEADER
#define RTKNAVI_HEADER

#include "MOOS/libMOOS/MOOSLib.h"

#include "MOOS/libMOOSGeodesy/MOOSGeodesy.h"
#include "MOOS/libMOOS/App/MOOSInstrument.h"

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <vector>
#include <deque>
#include <math.h>

class RTKNAVI: public CMOOSInstrument {
public:
    RTKNAVI();
    virtual ~RTKNAVI();

    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();

protected:
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::socket sock;

    boost::thread io_thread;
    boost::asio::deadline_timer timeout;

    std::vector<char> readBuffer;
    bool data_available;

    void ioLoop();
    void parseLine(std::string sline);

    void read_handler(bool& data_available,
            boost::asio::deadline_timer& timeout,
            const boost::system::error_code& error,
            std::size_t bytes_transferred);
    void wait_callback(boost::asio::ip::tcp::socket& tcp_sock,
            const boost::system::error_code& error);

    int asyncBytesRead;
    CMOOSGeodesy m_Geodesy;

    std::deque<double> vel_history, x_history, y_history;
    double x_prev, y_prev, time_prev;
};

#endif 
