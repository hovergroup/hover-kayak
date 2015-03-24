/*
 * 
 *        File: XBee.h
 *  Created on: Feb 21, 2015
 *      Author: Josh Leighton
 */

#ifndef XBee_HEADER
#define XBee_HEADER

#include "MOOS/libMOOS/MOOSLib.h"
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

class XBee : public CMOOSApp
{
public:
    XBee();
    ~XBee();

protected:
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();
    void RegisterVariables();

private:
    // basic serial port components
    boost::asio::io_service io;
    boost::asio::serial_port port;
    boost::asio::deadline_timer timeout;

    // constants
    static const std::string ack_start_string, ack_stop_string, var_start_string, var_stop_string;

    // configuration
    std::string m_name;
    bool m_weAreShoreside;

    // buffering
    int m_bufferIndex;
    std::vector<char> m_readBuffer, m_writeBuffer;

    // asio operations
    void read_handler(bool& data_available, boost::asio::deadline_timer& timeout,
                const boost::system::error_code& error, std::size_t bytes_transferred);
    void wait_callback(boost::asio::serial_port& ser_port, const boost::system::error_code& error);

    // reading
    void processBuffer();
    void handleAckFrom(std::string vname);
    void handleVar(std::string var) {}

    // shoreside specific stuff
    std::vector<std::string> m_knownVehicles;
    std::map<std::string, bool> m_vehicleStatus;
};

#endif 
