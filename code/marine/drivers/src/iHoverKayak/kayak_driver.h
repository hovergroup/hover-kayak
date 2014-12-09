/*
 * iHoverKayak
 *        File: kayak_driver.h
 *  Created on: Jun 20, 2012
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

class kayak_driver : public CMOOSApp
{
public:
	kayak_driver();
	virtual ~kayak_driver() {};

	bool OnNewMail(MOOSMSG_LIST &NewMail);
	bool Iterate();
	bool OnConnectToServer();
	bool OnStartUp();
	void RegisterVariables();

	void open_port( std::string port_name, int baudRate );
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
	void read_handler(bool& data_available, boost::asio::deadline_timer& timeout,
				const boost::system::error_code& error, std::size_t bytes_transferred);
	void wait_callback(boost::asio::serial_port& ser_port, const boost::system::error_code& error);
	void null_handler(const boost::system::error_code& error, std::size_t bytes_transferred) {};

	std::vector<char> readBuffer, writeBuffer;
	boost::mutex writeBufferMutex;
	int bytesToWrite, buffer_index;;

	// the background loop responsible for interacting with the serial port
	void serialLoop();
	void processWriteBuffer();

	void writeData( unsigned char *ptr, int length );
	void writeData( char *ptr, int length );
	void writeData( const char *ptr, int length );

	void shiftBuffer( int shift );
	int processBuffer();
	int findLine( int index );

	void parseVoltages( int index, int stopIndex );
	void parseTemperatures( int index, int stopIndex );
	void parseCurrents( int index, int stopIndex );
	void parseActuators( int index, int stopIndex );
	void parseSlow( int index, int stopIndex );
    
    double last_actuator_publish;

	// commands
	int m_desired_rudder, m_desired_thrust;
	bool newCommand;
	int RUDDER_OFFSET;
	double m_last_command_time;
	bool INVERT_RUDDER;

	bool m_usingFreewave;
	double m_radioSetTime, m_radioWaitTime;
	bool first_radio_message;

	int mapThrust( int thrust_command );
	int mapRudder( int rudder_command );
	int roundFloat( double val );
	void sendMotorCommands();
	void toggleCompassCalibration();
	void setRadioPower(bool freewave);

	void postRadioPowerIsLocked();
	void postRadioPowerIsUnlocked();
};

#endif
