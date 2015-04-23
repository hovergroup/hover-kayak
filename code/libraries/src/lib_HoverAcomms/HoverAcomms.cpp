/*
 * lib_HoverAcomms
 *        File: HoverAcomms.cpp
 *  Created on: Jan 3, 2013
 *      Author: Josh Leighton
 */

#include "HoverAcomms.h"

using namespace HoverAcomms;

bool AcommsTransmission::setRate(Rate r) {
	m_rate = r;
	if (r==MINI) {
		m_protobuf.set_type(goby::acomms::protobuf::ModemTransmission::DRIVER_SPECIFIC);
		m_protobuf.SetExtension(micromodem::protobuf::type, micromodem::protobuf::MICROMODEM_MINI_DATA);
	} else if (r==REMUS_LBL) {
		m_protobuf.set_type(goby::acomms::protobuf::ModemTransmission::DRIVER_SPECIFIC);
		m_protobuf.SetExtension(micromodem::protobuf::type, micromodem::protobuf::MICROMODEM_REMUS_LBL_RANGING);
	} else if (r==TWO_WAY_RANGING) {
		m_protobuf.set_type(goby::acomms::protobuf::ModemTransmission::DRIVER_SPECIFIC);
		m_protobuf.SetExtension(micromodem::protobuf::type, micromodem::protobuf::MICROMODEM_TWO_WAY_PING);
	} else {
		m_protobuf.set_type(goby::acomms::protobuf::ModemTransmission::DATA);
		m_protobuf.set_rate(m_rate);
		if (m_protobuf.HasExtension(micromodem::protobuf::type)) {
			m_protobuf.ClearExtension(micromodem::protobuf::type);
		}
	}
	fillData(getData());
	return true;
}

bool AcommsTransmission::setRate(int r) {
	// special rates
	if (r==100) {
		return setRate(MINI);
	} else if (r==101) {
		return setRate(REMUS_LBL);
	} else if (r==102) {
		return setRate(TWO_WAY_RANGING);
	}
	// standard rates
	if (r<0 || r>6) {
		return false;
	} else {
		return setRate(reverseRate(r));
	}
}

// pack data into frames
void AcommsTransmission::packMessage(std::string data) {
	int frame_size = frameSize();
	int frame_count = frameCount();

	// just a single frame
	if ( data.size() <= frame_size ) {
		m_protobuf.add_frame(data);
	} else { // multiple frames
		int filled_size = 0;
		// pack in full frames
		while ( data.size() > frame_size && m_protobuf.frame_size()<frame_count ) {
			m_protobuf.add_frame(data.data(), frame_size);
			data = data.substr(frame_size, data.size()-frame_size);
		}
		// fill up last frame or use remaining data
		if (data.size()>0 && m_protobuf.frame_size()<frame_count ) {
			int leftover = std::min(frame_size, (int) data.size());
			m_protobuf.add_frame(data.data(), leftover);
		}
	}
}

int AcommsTransmission::fillData(const char * data, int length) {
	return fillData(std::string(data, length));
}

int AcommsTransmission::fillData(const std::string & data) {
	m_protobuf.clear_frame();
	if (m_rate==MINI) {
		std::string data_copy = data;
		if (data_copy.size()==1) {
			char filler = 0x00;
			data_copy.insert(0,&filler,1);
		}
		m_protobuf.add_frame(data_copy.data(),2);
		m_protobuf.mutable_frame(0)->at(0) &= 0x1f;
		return 2;
	} else if (m_rate==REMUS_LBL || m_rate==TWO_WAY_RANGING) {
		return 1;
	} else if (FrameSizeMap.find(m_rate)==FrameSizeMap.end()) {
		return -1; // no rate defined
	} else {
		packMessage(data);
		return getData().size();
	}
}

std::string AcommsBase::getLoggableString() const {
	std::string publish_me = m_protobuf.DebugString();
	while (publish_me.find("\n") != std::string::npos) {
		publish_me.replace(publish_me.find("\n"), 1, "<|>");
	}
	return publish_me;
}

std::string AcommsBase::getData() const {
	std::string s;
	for (int i=0; i<getNumFrames(); i++) {
		s += m_protobuf.frame(i);
	}
	return s;
}

std::string AcommsBase::getHexData() const {
    std::stringstream ss;
    std::string data = getData();
    for ( int i=0; i<data.size(); i++ ) {
    	ss << std::hex << (int) data[i];
		if ( i < data.size()-1 )
			ss << ":";
    }
    return ss.str();
}

AcommsTransmission::AcommsTransmission(std::string data, Rate rate, int dest) {
	setDest(dest);
	setRate(rate);
	fillData(data);
}

bool AcommsBase::parseFromString(std::string msg) {
	if (msg.empty()) return false;

	goby::acomms::protobuf::ModemTransmission tmp;
	if(tmp.ParseFromString(msg)) {
		m_protobuf.Clear();
		m_protobuf.CopyFrom(tmp);
		return true;
	} else {
		return false;
	}
}

void AcommsBase::copyFromProtobuf(const goby::acomms::protobuf::ModemTransmission & proto) {
	m_protobuf.Clear();
	m_protobuf.CopyFrom(proto);
}

std::string AcommsReception::verify(bool & ok) {
	std::stringstream ss;
    if (getRate()==REMUS_LBL || getRate()==TWO_WAY_RANGING) {
        ok = true;
        return "";
    }

	// verify number of statistics against rate
	int num_stats = m_protobuf.ExtensionSize(micromodem::protobuf::receive_stat);

	if (getRate()==FSK0) {
		if (num_stats!=2) {
			ss << "FSK packet had " << num_stats << " receive statistics.";
			ok = false;
			return ss.str();
		}
	} else if (num_stats!=1) {
		ss << "Non-FSK packet had " << num_stats << " receive statistics.";
		ok = false;
		return ss.str();
	}

	micromodem::protobuf::ReceiveStatistics my_stat;
	if (getRate()==FSK0) {
		my_stat = getStatistics(1);
	} else {
		my_stat = getStatistics(0);
	}
	if (my_stat.number_frames() != m_protobuf.frame_size()) {
		ss << "Statistics # frames (" << my_stat.number_frames() <<
				") did not match found frames (" << m_protobuf.frame_size() << ")";
		ok = false;
		return ss.str();
	}
	if (my_stat.number_bad_frames()<0 || my_stat.number_bad_frames()>my_stat.number_frames()) {
		ss << "No. bad frames (" << my_stat.number_bad_frames() <<
				") did not fall within bounds [0 " <<
				my_stat.number_frames() << "]";
		ok = false;
		return ss.str();
	}

	ok = true;
	return "";
}

Rate AcommsReception::getRate() const {
	if (m_protobuf.type() == goby::acomms::protobuf::ModemTransmission::DRIVER_SPECIFIC) {
		if (m_protobuf.GetExtension(micromodem::protobuf::type)==micromodem::protobuf::MICROMODEM_REMUS_LBL_RANGING) {
			return REMUS_LBL;
		} else {
			return MINI;
		}
	}else
		return reverseRate(getStatistics(1).rate());
}

std::string AcommsReception::getFrame(unsigned int i) const {
	if (i>getNumFrames())
		return "";
	else
		return m_protobuf.frame(i);
}

ReceiptStatus AcommsReception::getStatus() const {
	if (getNumFrames()>0 && getNumBadFrames()==0) {
		return GOOD;
	} else if (getNumFrames()>0 && getNumBadFrames()<getNumFrames()) {
		return PARTIAL;
	} else {
		return BAD;
	}
}

std::string AcommsReception::getBadFrameListing() const {
	std::stringstream ss;
	for (int i=0; i<getNumBadFrames(); i++) {
		ss << m_protobuf.GetExtension(micromodem::protobuf::frame_with_bad_crc,i);
		if (i<getNumBadFrames()-1) ss << ",";
	}
	return ss.str();
}

bool AcommsReception::frameOkay(unsigned int i) const {
	for (int j=0; j<getNumBadFrames(); j++) {
		if (i == m_protobuf.GetExtension(micromodem::protobuf::frame_with_bad_crc, j))
			return false;
	}
	return true;
}

std::string AcommsReception::getAllFrames() const {
	std::string s;
	for (int i=0; i<getNumFrames(); i++) {
		s+=getFrame(i);
	}
	return s;
}

const micromodem::protobuf::ReceiveStatistics & AcommsReception::getStatistics(unsigned int i) const {
	if (i>=getNumStats()) {
		i=getNumStats()-1;
	}
	return m_protobuf.GetExtension(micromodem::protobuf::receive_stat, i);
}

bool AcommsReception::hasRanging() const {
	// check that extension is there
	if (m_protobuf.HasExtension(micromodem::protobuf::ranging_reply))
		return true;
	else
		return false;
}

double AcommsReception::getRangingTime() const {
	if (!hasRanging()) return -1;

	micromodem::protobuf::RangingReply ranging = m_protobuf.GetExtension(micromodem::protobuf::ranging_reply);
	if (ranging.one_way_travel_time_size()<1) {
		return -1;
	} else {
		return ranging.one_way_travel_time(0);
	}
}

std::vector<double> AcommsReception::getRemusLBLTimes() const {
	std::vector<double> v;
	if (getRate()!=REMUS_LBL) return v;
	if (m_protobuf.HasExtension(micromodem::protobuf::ranging_reply)) {
		micromodem::protobuf::RangingReply rr =
				m_protobuf.GetExtension(micromodem::protobuf::ranging_reply);
		if (rr.one_way_travel_time_size()==4) {
			for (int i=0; i<4; i++) {
				v.push_back(rr.one_way_travel_time(i));
				if (isnan(v[i])) v[i]=-1;
			}
		}
	}

	return v;
}
