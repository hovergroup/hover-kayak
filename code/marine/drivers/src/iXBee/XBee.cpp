/*
 * 
 *        File: XBee.cpp
 *  Created on: Feb 21, 2015
 *      Author: Josh Leighton
 */

#include <iterator>
#include "MBUtils.h"
#include "XBee.h"

using namespace std;
using namespace boost::asio;
using namespace boost::posix_time;

const string XBee::ack_start_string = "!1!";
const string XBee::ack_stop_string = "@2@";
const string XBee::var_start_string = "#3#";
const string XBee::var_stop_string = "$4$";

//---------------------------------------------------------
// Constructor

XBee::XBee() : port(io), timeout(io)
{
    m_readBuffer = vector<char>(1000,0);
    m_bufferIndex = 0;
}

//---------------------------------------------------------
// Destructor

XBee::~XBee()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool XBee::OnNewMail(MOOSMSG_LIST &NewMail)
{
    MOOSMSG_LIST::iterator p;

    for(p=NewMail.begin(); p!=NewMail.end(); p++) {
        CMOOSMsg &msg = *p;

#if 0 // Keep these around just for template
    string key   = msg.GetKey();
    string comm  = msg.GetCommunity();
    double dval  = msg.GetDouble();
    string sval  = msg.GetString(); 
    string msrc  = msg.GetSource();
    double mtime = msg.GetTime();
    bool   mdbl  = msg.IsDouble();
    bool   mstr  = msg.IsString();
#endif
    }

    return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool XBee::OnConnectToServer()
{
    // register for variables here
    // possibly look at the mission file?
    // m_MissionReader.GetConfigurationParam("Name", <string>);
    // m_Comms.Register("VARNAME", 0);

    RegisterVariables();
    return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool XBee::Iterate()
{
    if (!port.is_open())
        return true;

    if (m_bufferIndex > 800) {
        cout << "read buffer getting full, purging" << endl;
        m_readBuffer = vector<char>(1000,0);
        m_bufferIndex = 0;
    }

    if (m_writeBuffer.size() > 0) {
        port.write_some(buffer(m_writeBuffer, m_writeBuffer.size()));
        m_writeBuffer.resize(0);
    }

    int wait_time = 1000 / GetAppFreq();

    bool data_available;
    // set up an asynchronous read that will read up to 100 bytes, but will return as soon as any bytes area read
    // bytes read will be placed into readBuffer starting at index 0
    port.async_read_some( buffer( &m_readBuffer[m_bufferIndex], m_readBuffer.size()-m_bufferIndex ),
            boost::bind( &XBee::read_handler, this, boost::ref(data_available), boost::ref(timeout),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred ) );
    // setup a timer that will prevent the asynchronous operation for more than 100 ms
    timeout.expires_from_now( boost::posix_time::milliseconds(wait_time) );
    timeout.async_wait( boost::bind( &XBee::wait_callback, this, boost::ref(port),
            boost::asio::placeholders::error ) );

    // reset then run the io service to start the asynchronous operation
    io.reset();
    io.run();
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool XBee::OnStartUp()
{
    string port_name = "/dev/ttyUSB0";
    double baud_rate = 19200;

    m_MissionReader.GetConfigurationParam("baud_rate", baud_rate);
    m_MissionReader.GetConfigurationParam("port_name", port_name);
    m_MissionReader.GetValue("Community", m_name);
    MOOSToUpper(m_name);

    string mode;
    if (!m_MissionReader.GetConfigurationParam("mode", mode)) {
        cout << "Must set mode" << endl;
        return false;
    }
    MOOSToUpper(mode);
    if (mode == "SHORESIDE") {
        m_weAreShoreside = true;
    } else if (mode == "VEHICLE") {
        m_weAreShoreside = false;
    } else {
        cout << "Invalid mode: " << mode << endl;
        return false;
    }

    // open the serial port
    port.open(port_name);

    // serial port must be configured after being opened
    port.set_option(serial_port_base::baud_rate(baud_rate));
    port.set_option(serial_port_base::flow_control(serial_port_base::flow_control::none));
    port.set_option(serial_port_base::parity(serial_port_base::parity::none));
    port.set_option(serial_port_base::stop_bits(serial_port_base::stop_bits::one));
    port.set_option(serial_port_base::character_size(8));

    return true;
}

//---------------------------------------------------------
// Procedure: RegisterVariables

void XBee::RegisterVariables()
{
    // m_Comms.Register("FOOBAR", 0);
}

void XBee::read_handler(bool& data_available, deadline_timer& timeout,
    const boost::system::error_code& error, std::size_t bytes_transferred)
{
    if (error || !bytes_transferred) {
        return;
    }
    m_bufferIndex+=bytes_transferred;
    timeout.cancel();

    processBuffer();
}

void XBee::wait_callback(serial_port& ser_port, const boost::system::error_code& error)
{
    if (error) {
        // data read, timeout cancelled
        return;
    }
    port.cancel(); // read_callback fires with error
}

void XBee::processBuffer() {
    string string_buf(m_readBuffer.begin(), m_readBuffer.begin()+=m_bufferIndex);
    cout << "Processing " << string_buf.size() << " byte buffer." << endl;
    int max_index = 0;
    bool found = true;
    while (found) {
        int ack_stop = string_buf.find(ack_stop_string);
        int ack_start = string_buf.find(ack_start_string);
        if (ack_stop!=string::npos && ack_start!=string::npos && ack_start<ack_stop) {
            cout << "Found ack between " << ack_start << " and " << ack_stop << endl;
            string vname = string_buf.substr(ack_start + 3, ack_stop - ack_start - 3);
            MOOSToUpper(vname);
            cout << "Parsed ack from " << vname << endl;
            handleAckFrom(vname);
            string_buf.erase(ack_start, ack_stop-ack_start+3);
        } else {
            found = false;
        }
        if (ack_stop > max_index)
            max_index = ack_stop+2;
    }

    found = true;
    while (found) {
        int ack_stop = string_buf.find(var_stop_string);
        int ack_start = string_buf.find(var_start_string);
        if (ack_stop!=string::npos && ack_start!=string::npos && ack_start<ack_stop) {
            cout << "Found var between " << ack_start << " and " << ack_stop << endl;
            string var = string_buf.substr(ack_start + 3, ack_stop - ack_start - 3);
            handleVar(var);
            string_buf.erase(ack_start, ack_stop-ack_start+3);
        } else {
            found = false;
        }
    }

    m_readBuffer = std::vector<char>(string_buf.begin(), string_buf.end());
    m_bufferIndex = m_readBuffer.size();
    m_readBuffer.resize(1000,0);
}

void XBee::handleAckFrom(std::string vname) {
    if (m_weAreShoreside) {

    }
}
