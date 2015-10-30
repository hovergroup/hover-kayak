/*
 * iRTKNAVI
 *        File: RTKNAVI.cpp
 *  Created on: May 31, 2013
 *      Author: Josh Leighton
 */

#include <iterator>
#include "RTKNAVI.h"

using boost::asio::ip::tcp;
using namespace boost::asio;

//---------------------------------------------------------
// Constructor

RTKNAVI::RTKNAVI() :
        sock(io_service), timeout(io_service) {
    readBuffer = std::vector<char>(1000, 0);
    data_available = false;
    time_prev = -1;
}

//---------------------------------------------------------
// Destructor

RTKNAVI::~RTKNAVI() {
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool RTKNAVI::OnNewMail(MOOSMSG_LIST &NewMail) {
    MOOSMSG_LIST::iterator p;

    for (p = NewMail.begin(); p != NewMail.end(); p++) {
        CMOOSMsg &msg = *p;
    }

    return (true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool RTKNAVI::OnConnectToServer() {
    // register for variables here
    // possibly look at the mission file?
    // m_MissionReader.GetConfigurationParam("Name", <string>);
    // m_Comms.Register("VARNAME", is_float(int));

    return (true);
}

//---------------------------------------------------------
// Procedure: Iterate()

bool RTKNAVI::Iterate() {
    // happens AppTick times per second

    return (true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()

bool RTKNAVI::OnStartUp() {
    m_MissionReader.EnableVerbatimQuoting(false);

    // get lat and long origin from moos file
    double m_lat_origin, m_lon_origin;
    bool ok1, ok2;
    double * ptr = &m_lat_origin;
    ok1 = m_MissionReader.GetValue("LatOrigin", *ptr);
    ptr = &m_lon_origin;
    ok2 = m_MissionReader.GetValue("LongOrigin", *ptr);
    if (!ok1 || !ok2) {
        std::cout << "Error reading Lat/Long origin from MOOS file."
                << std::endl;
        return false;
    }

    // initialize geodesy
    if (!m_Geodesy.Initialise(m_lat_origin, m_lon_origin)) {
        std::cout << "Error initializing geodesy" << std::endl;
        return false;
    }

    std::string host, port;
    m_MissionReader.GetConfigurationParam("HOST", host);
    m_MissionReader.GetConfigurationParam("PORT", port);

    tcp::resolver resolver(io_service);
    tcp::resolver::query query(tcp::v4(), host, port);
    tcp::resolver::iterator iterator = resolver.resolve(query);

    sock.connect(*iterator);

    io_thread = boost::thread(boost::bind(&RTKNAVI::ioLoop, this));

    return (true);
}

void RTKNAVI::parseLine(std::string sline) {
    if (sline.size() < 2)
        return;
    std::string date = MOOSChomp(sline, " ");
    date += " " + MOOSChomp(sline, "  ");
//	std::cout << date << std::endl;
    try {
        boost::posix_time::ptime t(boost::posix_time::time_from_string(date));
        m_Comms.Notify("RTK_PTIME", boost::posix_time::to_simple_string(t));
        double seconds = t.time_of_day().total_milliseconds() / 1000.0;
        m_Comms.Notify("RTK_TIME_SECONDS", seconds);
    } catch (...) {
        std::cout << "ptime exception" << std::endl;
        return;
    }

    double lat = atof(MOOSChomp(sline, "  ").c_str());
    m_Comms.Notify("RTK_LATITUDE", lat);
    double lon = atof(MOOSChomp(sline, "  ").c_str());
    m_Comms.Notify("RTK_LONGITUDE", lon);
    double height = atof(MOOSChomp(sline, "  ").c_str());
    m_Comms.Notify("RTK_HEIGHT", height);
    int quality = atoi(MOOSChomp(sline, "  ").c_str());
    m_Comms.Notify("RTK_QUALITY", quality);
    int ns = atoi(MOOSChomp(sline, "  ").c_str());
    m_Comms.Notify("RTK_NUM_SV", ns);

    double dfXLocal, dfYLocal;
    if (m_Geodesy.LatLong2LocalUTM(lat, lon, dfYLocal, dfXLocal)) {
        m_Comms.Notify("RTK_X", dfXLocal);
        m_Comms.Notify("RTK_Y", dfYLocal);

	double time_curr = MOOSTime();
        if (time_prev != -1) {
            double vel = sqrt(
                    pow(dfXLocal - x_prev, 2) + pow(dfYLocal - y_prev, 2))
                    / (time_curr - time_prev);
            vel_history.push_back(vel);

            x_history.push_back(dfXLocal);
            y_history.push_back(dfYLocal);


            if (vel_history.size() >= 5) {
                double sum = 0;
                for (int i = 0; i < vel_history.size(); i++) {
                    sum += vel_history[i];
                }
                m_Comms.Notify("RTK_SPEED", sum / vel_history.size());
                vel_history.pop_front();

                double heading = 180/M_PI *
                        atan2(x_history.back() - x_history.front(),
                                y_history.back() - y_history.front());
                x_history.pop_front();
                y_history.pop_front();
                while (heading < 0) heading += 360;
                while (heading > 360) heading -= 360;
                m_Comms.Notify("RTK_HEADING", heading);
            }
        }
        x_prev = dfXLocal;
        y_prev = dfYLocal;
        time_prev = time_curr;
    }
}

void RTKNAVI::ioLoop() {
    while (true) {
        // set up an asynchronous read that will read up to 100 bytes, but will return as soon as any bytes area read
        // bytes read will be placed into readBuffer starting at index 0
        sock.async_read_some(buffer(&readBuffer[0], 1000),
                boost::bind(&RTKNAVI::read_handler, this,
                        boost::ref(data_available), boost::ref(timeout),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
        // setup a timer that will prevent the asynchronous operation for more than 100 ms
        timeout.expires_from_now(boost::posix_time::milliseconds(1000));
        timeout.async_wait(
                boost::bind(&RTKNAVI::wait_callback, this, boost::ref(sock),
                        boost::asio::placeholders::error));

        // reset then run the io service to start the asynchronous operation
        io_service.reset();
        io_service.run();

        if (data_available) {
            parseLine(std::string(readBuffer.begin(), readBuffer.begin() +=
                    asyncBytesRead));
        }
    }
}

void RTKNAVI::read_handler(bool& data_available, deadline_timer& timeout,
        const boost::system::error_code& error, std::size_t bytes_transferred) {
//	if (error || bytes_transferred<=1) {
    if (error || !bytes_transferred) {
        // no data read
        data_available = false;
        return;
    }
    data_available = true;
    asyncBytesRead = bytes_transferred;
    timeout.cancel();
}

void RTKNAVI::wait_callback(tcp::socket& ser_port,
        const boost::system::error_code& error) {
    if (error) {
        // data read, timeout cancelled
        return;
    }
    data_available = false;
    ser_port.cancel(); // read_callback fires with error
}
