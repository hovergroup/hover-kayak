/*
 * lib_HoverAcomms
 *        File: HoverAcomms.h
 *  Created on: Jan 3, 2013
 *      Author: Josh Leighton
 */

#ifndef LIB_HOVERACOMMS_H_
#define LIB_HOVERACOMMS_H_

#include "goby/acomms/modem_driver.h"
#include "goby/acomms/protobuf/mm_driver.pb.h"
#include <string>
#include <map>
#include "boost/assign.hpp"
#include "MOOS/libMOOS/MOOSLib.h"

namespace HoverAcomms {

enum DriverStatus {
	READY = 0,
	TRANSMITTING,
	RECEIVING,
	NOT_RUNNING,
	STARTING
};

enum Rate {
	FSK0 = 0,
	PSK1,
	PSK2,
	PSK3,
	PSK4,
	PSK5,
	PSK6,
	MINI,
	REMUS_LBL,
	TWO_WAY_RANGING
};

enum ReceiptStatus {
	GOOD = 0, // all frames received good
	PARTIAL, // some frames received good, some bad
	BAD // no frames received
};

static const std::map<int,Rate> ReverseRateMap = boost::assign::map_list_of
		(0,FSK0)
		(1,PSK1)
		(2,PSK2)
		(3,PSK3)
		(4,PSK4)
		(5,PSK5)
		(6,PSK6)
		(100,MINI)
		(101,REMUS_LBL)
		(102,TWO_WAY_RANGING);

static const std::map<Rate,int> FrameSizeMap = boost::assign::map_list_of
		(FSK0,32)
		(PSK1,64)
		(PSK2,64)
		(PSK3,256)
		(PSK4,256)
		(PSK5,256)
		(PSK6,32);
static const std::map<Rate,int> FrameCountMap = boost::assign::map_list_of
		(FSK0,1)
		(PSK1,3)
		(PSK2,3)
		(PSK3,2)
		(PSK4,2)
		(PSK5,8)
		(PSK6,6);

class AcommsBase {
public:
	std::string getLoggableString() const;

	std::string serialize() { return m_protobuf.SerializeAsString(); }
	std::string serializeWithInfo() {
		std::stringstream ss;
		ss << "vname=" << m_vehicleName <<
			  ":time=" << m_time <<
			  ":loc=" << m_navx << "," << m_navy << ":" <<
			  serialize();
		return ss.str();
	};

	bool parseFromString(std::string msg);
	void copyFromProtobuf(const goby::acomms::protobuf::ModemTransmission & proto);

	int getNumFrames() const { return m_protobuf.frame_size(); }

	std::string getHexData() const;
	std::string getData() const;

	std::string m_vehicleName;
	double m_time, m_navx, m_navy;

	goby::acomms::protobuf::ModemTransmission m_protobuf;

protected:
	Rate reverseRate(int i) const { return ReverseRateMap.find(i)->second; }
};

class AcommsReception : public AcommsBase {
public:
	AcommsReception() {}

	std::string verify(bool & ok);

	int getSource() const { return m_protobuf.src(); }
	int getDest() const { return m_protobuf.dest(); }
	int getNumStats() const { return m_protobuf.ExtensionSize(micromodem::protobuf::receive_stat); }
	int getNumBadFrames() const { return m_protobuf.ExtensionSize(micromodem::protobuf::frame_with_bad_crc); }

	bool hasRanging() const;
	double getRangingTime() const;

	ReceiptStatus getStatus() const;

	Rate getRate() const;
	std::string getFrame(unsigned int i) const;
	bool frameOkay(unsigned int i) const;
	std::string getAllFrames() const;

	std::string getBadFrameListing() const;

	std::vector<double> getRemusLBLTimes() const;

	const micromodem::protobuf::ReceiveStatistics & getStatistics(unsigned int i=0) const;

protected:

};

class AcommsTransmission : public AcommsBase {
public:
	AcommsTransmission() { m_rate = FSK0; }
	AcommsTransmission(std::string data, Rate rate, int dest=0);

	bool setRate(Rate r);
	bool setRate(int r);
	void setDest(int d) { m_protobuf.set_dest(d); }
	void setAckRequested(bool b) { m_protobuf.set_ack_requested(b); }

	int fillData(const char * data, int length);
	int fillData(const std::string & data);

	Rate 			getRate() const	{ return m_rate; }
	int				getDest() const { return m_protobuf.dest(); }

	const goby::acomms::protobuf::ModemTransmission & getProtobuf() const {return m_protobuf;}

protected:
	Rate m_rate;

	void packMessage(std::string data);

	int frameSize() {
		if(FrameSizeMap.find(m_rate)==FrameSizeMap.end())
			return -1;
		else
			return FrameSizeMap.find(m_rate)->second; }
	int frameCount() {
		if(FrameCountMap.find(m_rate)==FrameCountMap.end())
			return -1;
		else
			return FrameCountMap.find(m_rate)->second; }
};

};

#endif // LIB_HOVERACOMMS_H_
