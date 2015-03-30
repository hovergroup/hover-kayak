/*
 * iOS5000_Hover
 *        File: iOS5000.cpp
 *  Created on: Aug 9, 2012
 *      Author: Josh Leighton
 */

#include <iterator>
#include "iOS5000.h"
#include "MBUtils.h"

using namespace std;
using namespace boost::asio;
using namespace boost::posix_time;

iOS5000_Hover::iOS5000_Hover() :
        port(io), timeout(io) {
    // initialize buffers with some size larger than should ever be needed
    writeBuffer = vector<unsigned char>(1000, 0);
    readBuffer = vector<unsigned char>(1000, 0);

    bytesToWrite = 0;
    asyncBytesRead = 0;
    data_available = false;
    stop_requested = false;

    my_baud_rate = 115200;
    my_port_name = "/dev/ttyUSB0";

    string_buffer = "";

    current_x_estimate = 0;
    current_y_estimate = 0;
    filter_constant = 1;
    prerotation = 0;

    last_msg_time = 0;
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool iOS5000_Hover::OnNewMail(MOOSMSG_LIST &NewMail) {
    return (true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool iOS5000_Hover::OnConnectToServer() {
    STRING_LIST sParams;
    m_MissionReader.EnableVerbatimQuoting(false);
    m_MissionReader.GetConfiguration(GetAppName(), sParams);

    // port name and baud rate
    m_MissionReader.GetConfigurationParam("Speed", my_baud_rate);
    m_MissionReader.GetConfigurationParam("Port", my_port_name);
    m_MissionReader.GetConfigurationParam("PreRotation", prerotation);
    m_MissionReader.GetConfigurationParam("FilterTimeConstant",
            filter_constant);

    RegisterVariables();

    open_port(my_port_name, my_baud_rate);
    //serial_thread = boost::thread(boost::bind(&iOS5000_Hover::serialLoop, this));

    return (true);
}

//------------------------------------------------------------
// Procedure: RegisterVariables

void iOS5000_Hover::RegisterVariables() {
}

//---------------------------------------------------------
// Procedure: Iterate()

bool iOS5000_Hover::Iterate() {
    return (true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//      Note: happens before connection is open

bool iOS5000_Hover::OnStartUp() {
    // I prefer to do nothing here
    return (true);
}

void iOS5000_Hover::open_port(string port_name, int baudRate) {
    if (port.is_open())
        return;
    // open the serial port
    cout << "Opening " << port_name << endl;
    port.open(port_name);

    // serial port must be configured after being opened
    port.set_option(serial_port_base::baud_rate(baudRate));
    port.set_option(
            serial_port_base::flow_control(
                    serial_port_base::flow_control::none));
    port.set_option(serial_port_base::parity(serial_port_base::parity::none));
    port.set_option(
            serial_port_base::stop_bits(serial_port_base::stop_bits::one));
    port.set_option(serial_port_base::character_size(8));

    // start the background thread
    serial_thread = boost::thread(
            boost::bind(&iOS5000_Hover::serialLoop, this));
}

void iOS5000_Hover::close_port() {
    stop_requested = true;
    serial_thread.join();
    port.close();
}

void iOS5000_Hover::writeData(unsigned char *ptr, int length) {
    writeBufferMutex.lock();
    memcpy(&writeBuffer[bytesToWrite], ptr, length);
    bytesToWrite += length;
    writeBufferMutex.unlock();
}

void iOS5000_Hover::read_handler(bool& data_available, deadline_timer& timeout,
        const boost::system::error_code& error, std::size_t bytes_transferred) {
    if (error || !bytes_transferred) {
        // no data read
        data_available = false;
        return;
    }
    data_available = true;
    asyncBytesRead = bytes_transferred;
    timeout.cancel();
}

void iOS5000_Hover::wait_callback(serial_port& ser_port,
        const boost::system::error_code& error) {
    if (error) {
        // data read, timeout cancelled
        return;
    }
    port.cancel(); // read_callback fires with error
}

void iOS5000_Hover::processWriteBuffer() {
    // take out lock
    writeBufferMutex.lock();
    if (bytesToWrite > 0) {
        // if there is data waiting, copy it to a local buffer
        vector<unsigned char> localWriteBuffer(bytesToWrite, 0);
        memcpy(&localWriteBuffer[0], &writeBuffer[0], bytesToWrite);
        bytesToWrite = 0;
        // release lock to prevent outside write requests from blocking on serial write
        writeBufferMutex.unlock();

        cout << endl << dec << "writing " << localWriteBuffer.size() << " bytes"
                << endl;

        // simple synchronous serial write
        port.write_some(buffer(localWriteBuffer, localWriteBuffer.size()));
    } else {
        // no data to write, release lock
        writeBufferMutex.unlock();
    }
}

void iOS5000_Hover::processLine(double heading, double pitch, double roll,
        double temp) {
    heading += prerotation;
    while (heading > 360)
        heading -= 360.0;
    while (heading < 0)
        heading += 360.0;

    m_Comms.Notify("COMPASS_HEADING_UNFILTERED", heading);

    double dT = MOOSTime() - last_msg_time;
    last_msg_time = MOOSTime();
    double alpha = 1.0 / (1.0 + filter_constant / dT);

    double new_x = sin(heading * M_PI / 180.0);
    double new_y = cos(heading * M_PI / 180.0);
    current_x_estimate = alpha * new_x + (1 - alpha) * current_x_estimate;
    current_y_estimate = alpha * new_y + (1 - alpha) * current_y_estimate;
    double heading_estimate = atan2(current_x_estimate, current_y_estimate)
            * 180.0 / M_PI;

    while (heading_estimate > 360)
        heading_estimate -= 360.0;
    while (heading_estimate < 0)
        heading_estimate += 360.0;

    m_Comms.Notify("COMPASS_HEADING_FILTERED", heading_estimate);
    m_Comms.Notify("COMPASS_PITCH", pitch);
    m_Comms.Notify("COMPASS_ROLL", roll);
    m_Comms.Notify("COMPASS_TEMPERATURE", temp);
}

void iOS5000_Hover::parseLine(string msg) {
//	cout << "parsing line: " << msg << endl;
    // $C320.5P0.2R-18.3T19.0*3C
    if (msg[0] != '$' || msg[1] != 'C')
        return;

    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;

    vector<string> subs;

    boost::char_separator<char> sep("$CPRT*");
    tokenizer tok(msg, sep);
    for (tokenizer::iterator beg = tok.begin(); beg != tok.end(); ++beg) {
        subs.push_back(*beg);
    }

    if (subs.size() < 4)
        return;

    double heading = atof(subs[0].c_str());
    double pitch = atof(subs[1].c_str());
    double roll = atof(subs[2].c_str());
    double temp = atof(subs[3].c_str());

    processLine(heading, pitch, roll, temp);
}

void iOS5000_Hover::serialLoop() {
    while (!stop_requested) {

        // processWriteBuffer();

        // set up an asynchronous read that will read up to 100 bytes, but will return as soon as any bytes area read
        // bytes read will be placed into readBuffer starting at index 0
        port.async_read_some(buffer(&readBuffer[0], 1000),
                boost::bind(&iOS5000_Hover::read_handler, this,
                        boost::ref(data_available), boost::ref(timeout),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
        // setup a timer that will prevent the asynchronous operation for more than 100 ms
        timeout.expires_from_now(boost::posix_time::milliseconds(1000));
        timeout.async_wait(
                boost::bind(&iOS5000_Hover::wait_callback, this,
                        boost::ref(port), boost::asio::placeholders::error));

        // reset then run the io service to start the asynchronous operation
        io.reset();
        io.run();

        if (data_available) {
            string_buffer += string(readBuffer.begin(), readBuffer.begin() +=
                    asyncBytesRead);
//			cout << string_buffer << endl;
            while (string_buffer.find("\n", 1) != string::npos) {
                int index = string_buffer.find("\n", 1);
                parseLine(string_buffer.substr(0, index));
//				cout << "index: " << index << endl;
//				cout << string_buffer.substr(0, index) << endl;
//				m_Comms.Notify("GPS_SENTENCE", string_buffer.substr(0, index) );
                string_buffer = string_buffer.substr(index + 1,
                        string_buffer.size() - index - 1);
            }
            // print out read data in hexidecimal format
            /*for (int i=0; i<asyncBytesRead; i++) {
             cout << readBuffer[i];
             }
             cout.flush();*/
        }
    }
}
