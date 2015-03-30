/*
 * iGPS_Hover
 *        File: simple_gps.cpp
 *  Created on: Jul 24, 2012
 *      Author: Josh Leighton
 */

#include <iterator>
#include "simple_gps.h"
#include "MBUtils.h"

using namespace std;
//using namespace boost::asio;
using namespace boost::posix_time;

SIMPLE_GPS::SIMPLE_GPS() :
        port(io), timeout(io) {
    // initialize buffers with some size larger than should ever be needed
    writeBuffer = vector<unsigned char>(1000, 0);
    readBuffer = vector<unsigned char>(1000, 0);
    tcpReadBuffer = vector<char>(1000, 0);

    bytesToWrite = 0;
    asyncBytesRead = 0;
    data_available = false;
    stop_requested = false;

    my_baud_rate = 38400;
    my_port_name = "/dev/ttyUSB0";

    string_buffer = "";

    driver_initialized = false;
    m_use_tcp = false;
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool SIMPLE_GPS::OnNewMail(MOOSMSG_LIST &NewMail) {
    // if using tcp, assume we're logging gps server side
    if (m_use_tcp)
        return true;

    MOOSMSG_LIST::iterator p;

    for (p = NewMail.begin(); p != NewMail.end(); p++) {
        CMOOSMsg &msg = *p;
        string key = msg.GetKey();

        if (key == "LOGGER_DIRECTORY" && !driver_initialized) {
            driver_initialized = true;
            std::string logDirectory = msg.GetString();

            cout << "opening gps log file..." << endl;
            int file_index = 0;
            std::string filename = logDirectory + "/gps_log_"
                    + boost::lexical_cast<string>(file_index) + ".txt";
            while (file_exists(filename)) {
                cout << filename << " already exists." << endl;
                file_index++;
                filename = logDirectory + "/gps_log_"
                        + boost::lexical_cast<string>(file_index) + ".txt";
            }
            m_gps_log.open(filename.c_str());

            // get log directory from plogger that we will also use
            open_port(my_port_name, my_baud_rate);
        }
    }

    return true;
}

// check if a file exists
bool SIMPLE_GPS::file_exists(std::string filename) {
    ifstream my_file(filename.c_str());
    if (my_file.good()) {
        my_file.close();
        return true;
    } else {
        my_file.close();
        return false;
    }
}

void SIMPLE_GPS::writeLine(std::string sLine) {
    boost::posix_time::ptime pt(
            boost::posix_time::microsec_clock::universal_time());
    m_gps_log << boost::posix_time::to_simple_string(pt) << ":  ";
    m_gps_log << sLine << endl;
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool SIMPLE_GPS::OnConnectToServer() {
    STRING_LIST sParams;
    m_MissionReader.EnableVerbatimQuoting(false);
    m_MissionReader.GetConfiguration(GetAppName(), sParams);

    // get lat and long origin from moos file
    double m_lat_origin, m_lon_origin;
    bool ok1, ok2;
    double * ptr = &m_lat_origin;
    ok1 = m_MissionReader.GetValue("LatOrigin", *ptr);
    ptr = &m_lon_origin;
    ok2 = m_MissionReader.GetValue("LongOrigin", *ptr);
    if (!ok1 || !ok2) {
        cout << "Error reading Lat/Long origin from MOOS file." << endl;
        return false;
    }

    // initialize geodesy
    if (!m_Geodesy.Initialise(m_lat_origin, m_lon_origin)) {
        cout << "Error initializing geodesy" << endl;
        return false;
    }

    // port name and baud rate
    m_MissionReader.GetConfigurationParam("BAUD_RATE", my_baud_rate);
    m_MissionReader.GetConfigurationParam("PORT_NAME", my_port_name);

    std::string server_name;
    int server_port;
    m_MissionReader.GetConfigurationParam("TCP_SERVER", server_name);
    m_MissionReader.GetConfigurationParam("TCP_PORT", server_port);

    m_MissionReader.GetConfigurationParam("USE_TCP", m_use_tcp);

    if (m_use_tcp) {
        struct sockaddr_in m_server_address;
        struct hostent *m_server;

        m_tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (m_tcp_sockfd < 0) {
            std::cout << "Error opening socket" << std::endl;
            return false;
        }

        m_server = gethostbyname(server_name.c_str());
        if (m_server == NULL) {
            std::cout << "Error, no such host " << server_name << std::endl;
            return false;
        }

        bzero((char *) &m_server_address, sizeof(m_server_address));
        m_server_address.sin_family = AF_INET;
        bcopy((char *) m_server->h_addr,
        (char *)&m_server_address.sin_addr.s_addr,
        m_server->h_length);
        m_server_address.sin_port = htons(server_port);
        if (connect(m_tcp_sockfd, (struct sockaddr *) &m_server_address,
                sizeof(m_server_address)) < 0) {
            std::cout << "error connecting" << std::endl;
            return false;
        }
    } else {
        RegisterVariables();
    }

//	open_port( my_port_name, my_baud_rate );
    //serial_thread = boost::thread(boost::bind(&SIMPLE_GPS::serialLoop, this));

    return (true);
}

//------------------------------------------------------------
// Procedure: RegisterVariables

void SIMPLE_GPS::RegisterVariables() {
    m_Comms.Register("LOGGER_DIRECTORY", 1);
}

//---------------------------------------------------------
// Procedure: Iterate()

bool SIMPLE_GPS::Iterate() {
    if (m_use_tcp) {
        int n = recv(m_tcp_sockfd, &tcpReadBuffer[0], 1000, 0);
//        if (n < 0) {
//            std::cout << "error reading from socket" << std::endl;
//            return false;
//        }
        if (n > 0) {
            string_buffer += std::string(tcpReadBuffer.begin(),
                    tcpReadBuffer.begin() += n);
        }
        processReadBuffer();
    }

    return (true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//      Note: happens before connection is open

bool SIMPLE_GPS::OnStartUp() {
    // I prefer to do nothing here
    return (true);
}

void SIMPLE_GPS::open_port(string port_name, int baudRate) {
    if (port.is_open())
        return;
    // open the serial port
    cout << "Opening " << port_name << endl;
    port.open(port_name);

    // serial port must be configured after being opened
    port.set_option(boost::asio::serial_port_base::baud_rate(baudRate));
    port.set_option(
            boost::asio::serial_port_base::flow_control(
                    boost::asio::serial_port_base::flow_control::none));
    port.set_option(
            boost::asio::serial_port_base::parity(
                    boost::asio::serial_port_base::parity::none));
    port.set_option(
            boost::asio::serial_port_base::stop_bits(
                    boost::asio::serial_port_base::stop_bits::one));
    port.set_option(boost::asio::serial_port_base::character_size(8));

    // start the background thread
    serial_thread = boost::thread(boost::bind(&SIMPLE_GPS::serialLoop, this));
}

void SIMPLE_GPS::close_port() {
    stop_requested = true;
    serial_thread.join();
    port.close();
}

void SIMPLE_GPS::writeData(unsigned char *ptr, int length) {
    writeBufferMutex.lock();
    memcpy(&writeBuffer[bytesToWrite], ptr, length);
    bytesToWrite += length;
    writeBufferMutex.unlock();
}

void SIMPLE_GPS::read_handler(bool& data_available,
        boost::asio::deadline_timer& timeout,
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

void SIMPLE_GPS::wait_callback(boost::asio::serial_port& ser_port,
        const boost::system::error_code& error) {
    if (error) {
        // data read, timeout cancelled
        return;
    }
    port.cancel(); // read_callback fires with error
}

void SIMPLE_GPS::processWriteBuffer() {
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
        port.write_some(
                boost::asio::buffer(localWriteBuffer, localWriteBuffer.size()));
    } else {
        // no data to write, release lock
        writeBufferMutex.unlock();
    }
}

void SIMPLE_GPS::serialLoop() {
    while (!stop_requested) {

        processWriteBuffer();

        // set up an asynchronous read that will read up to 100 bytes, but will return as soon as any bytes area read
        // bytes read will be placed into readBuffer starting at index 0
        port.async_read_some(boost::asio::buffer(&readBuffer[0], 1000),
                boost::bind(&SIMPLE_GPS::read_handler, this,
                        boost::ref(data_available), boost::ref(timeout),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
        // setup a timer that will prevent the asynchronous operation for more than 100 ms
        timeout.expires_from_now(boost::posix_time::milliseconds(1000));
        timeout.async_wait(
                boost::bind(&SIMPLE_GPS::wait_callback, this, boost::ref(port),
                        boost::asio::placeholders::error));

        // reset then run the io service to start the asynchronous operation
        io.reset();
        io.run();

//		cout << "read " << asyncBytesRead << endl;

        if (data_available) {
            string_buffer += string(readBuffer.begin(), readBuffer.begin() +=
                    asyncBytesRead);
//			cout << string_buffer << endl;

            // clear the string buffer if if it doesn't contain the start of a sentence
//            if (string_buffer.size()>3 && string_buffer.find("$GP", 0)==string::npos) {
//            	string_buffer = "";
//            }

            // look for a sentence if buffer is full enough
            if (string_buffer.size() > 20) {
                processReadBuffer();
            }
        }
    }
}

void SIMPLE_GPS::processReadBuffer() {
    int start_index, stop_index;
    while (1) {
        if ((start_index = string_buffer.find("$GP", 0)) != string::npos) {
            if ((stop_index = string_buffer.find("*", start_index + 2))
                    != string::npos) {
//				std::cout << "pre buffer: " << string_buffer << std::endl << std::endl;
//				std::cout << "stop index " << dec << stop_index << ", ";
//				std::cout << "start index " << start_index << std::endl << std::endl;
                stop_index += 3;
//				std::cout << "running parse on: " << string_buffer.substr(start_index, stop_index-start_index) << std::endl << std::endl;
                parseLine(
                        string_buffer.substr(start_index,
                                stop_index - start_index));
                string_buffer = string_buffer.substr(stop_index,
                        string_buffer.size() - stop_index);
//				std::cout << "post buffer: " << string_buffer << std::endl << std::endl;
            } else {
                break;
            }
        } else {
            break;
        }
    }

}
