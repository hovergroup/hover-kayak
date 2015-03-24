/*
 * iIcarus
 *        File: icarus.cpp
 *  Created on: Mar 11, 2014
 *      Author: Josh Leighton
 */

#include <iterator>
#include "icarus.h"
#include "MBUtils.h"

using namespace std;
using namespace boost::asio;
using namespace boost::posix_time;

icarus_driver::icarus_driver() :
        port(io), timeout(io) {
    readBuffer = vector<char>(1000, 0);

    buffer_index = 0;
    data_available = false;
    stop_requested = false;

    my_baud_rate = 9600;
    my_port_name = "/dev/ttyO1";
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool icarus_driver::OnNewMail(MOOSMSG_LIST &NewMail) {
    MOOSMSG_LIST::iterator p;
    for (p = NewMail.begin(); p != NewMail.end(); p++) {
        CMOOSMsg &msg = *p;
        string key = msg.GetKey();
    }

    return (true);
}

//---------------------------------------------------------
// Procedure: Iterate()

bool icarus_driver::Iterate() {
    return (true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool icarus_driver::OnConnectToServer() {
    m_MissionReader.GetConfigurationParam("BAUD_RATE", my_baud_rate);
    m_MissionReader.GetConfigurationParam("PORT_NAME", my_port_name);

    RegisterVariables();

    open_port(my_port_name, my_baud_rate);

    return (true);
}

//------------------------------------------------------------
// Procedure: RegisterVariables

void icarus_driver::RegisterVariables() {
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//      Note: happens before connection is open

bool icarus_driver::OnStartUp() {
    // I prefer to do nothing here
    return (true);
}

void icarus_driver::open_port(string port_name, int baudRate) {
//	cout << "Opening " << port_name << " at " << baudRate << endl;

// open the serial port
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
            boost::bind(&icarus_driver::serialLoop, this));
}

void icarus_driver::close_port() {
    stop_requested = true;
    serial_thread.join();
    port.close();
}

void icarus_driver::read_handler(bool& data_available, deadline_timer& timeout,
        const boost::system::error_code& error, std::size_t bytes_transferred) {
    if (error || !bytes_transferred) {
        // no data read
//		cout << "no data" << endl;
        data_available = false;
        return;
    }
//	cout << "data available: " << bytes_transferred << endl;
    data_available = true;
    buffer_index += bytes_transferred;
    timeout.cancel();
}

void icarus_driver::wait_callback(serial_port& ser_port,
        const boost::system::error_code& error) {
    if (error) {
        // data read, timeout cancelled
        return;
    }
    port.cancel(); // read_callback fires with error
}

int icarus_driver::findLine(int index) {
    for (int i = index; i < buffer_index; i++) {
        if (readBuffer[i] == '\r')
            return i;
    }
    return -1;
}

int icarus_driver::processBuffer() {
//	cout << "processing: ";
//	for (int i=0; i<buffer_index; i++ ) {
//		cout << readBuffer[i];
//	}
//	cout << endl;

    int bytesUsed = 0;
    int stopIndex = findLine(bytesUsed);
    if (stopIndex == -1)
        return bytesUsed;
    while (bytesUsed < buffer_index) {
        if (bytesUsed > stopIndex) {
            stopIndex = findLine(bytesUsed);
            if (stopIndex == -1)
                return bytesUsed;
        }
        switch (readBuffer[bytesUsed]) {
        case 'I':
            parseIcarus(bytesUsed, stopIndex);
            bytesUsed = stopIndex;
            break;
        }
        bytesUsed++;
    }
    return bytesUsed;
}

void icarus_driver::parseIcarus(int index, int stopIndex) {
    if (readBuffer[index] == 'I' && readBuffer[index + 1] == '=') {
        int battery_voltage, temperature;
        sscanf(&readBuffer[index], "I=%d,%d", &battery_voltage, &temperature);

        m_Comms.Notify("VOLTAGE", battery_voltage / 100.0);
        m_Comms.Notify("TEMPERATURE", temperature / 10.0);
    } else {
        cout << "bad parse" << endl;
    }
}

void icarus_driver::shiftBuffer(int shift) {
//	cout << "shifting " << buffer_index-shift << " bytes by " << shift << endl;

    if (shift == 0 || buffer_index == 0)
        return;
    for (int i = shift; i < buffer_index; i++) {
        readBuffer[i - shift] = readBuffer[i];
    }
    buffer_index -= shift;
}

void icarus_driver::serialLoop() {
    while (!stop_requested) {
//		cout << "async start" << endl;
        // set up an asynchronous read that will read up to 100 bytes, but will return as soon as any bytes area read
        // bytes read will be placed into readBuffer starting at index 0
        port.async_read_some(
                buffer(&readBuffer[buffer_index], 1000 - buffer_index),
                boost::bind(&icarus_driver::read_handler, this,
                        boost::ref(data_available), boost::ref(timeout),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
        // setup a timer that will prevent the asynchronous operation for more than 100 ms
        timeout.expires_from_now(boost::posix_time::milliseconds(1000));
        timeout.async_wait(
                boost::bind(&icarus_driver::wait_callback, this,
                        boost::ref(port), boost::asio::placeholders::error));

        // reset then run the io service to start the asynchronous operation
        io.reset();
        io.run();

//		cout << "async end" << endl;

        if (data_available) {
            shiftBuffer(processBuffer());
        }
    }
}
