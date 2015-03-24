/*
 * acommsParse
 *        File: acommsParse.cpp
 *  Created on: May 23, 2013
 *      Author: Josh Leighton
 */

#include "goby/acomms/protobuf/mm_driver.pb.h"
#include <google/protobuf/text_format.h>
#include "acommsParse.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <math.h>
#include <deque>

#define MAX_LINE_LENGTH 10000
ALogEntry getNextRawALogEntry_josh(FILE *fileptr, bool allstrings = false);
std::string readCommandArg( std::string sarg );
bool wildCardMatch( std::string wild, std::string key );

// functions copied from moos-ivp to support independent compilation
std::string biteString(std::string& str, char separator);
std::string stripBlankEnds(const std::string& str);
bool isNumber(const std::string& str, bool blanks_allowed=true);
bool scanArgs(int, char**, const char*, const char *a=0, const char *b=0);
bool strContains(const std::string& str, const std::string& qstr);

using namespace std;

vector<string> variables, wilds;
ofstream output;
string delimiter = ",";
deque<ALogEntry> hex_data_entries;
string hex_data_name;

enum MODE {
	UNSET,
	RECEIVE,
	TRANSMIT
};
MODE mode = UNSET;

void printHelp() {
    cout << "Usage: " << endl;
    cout << "  acommsParse in.alog [--receive / --transmit] [VAR] [OPTIONS    " << endl;
    cout << "                                                                 " << endl;
    cout << "Synopsis:                                                        " << endl;
    cout << "  Creates a comma delimited synchronous log file from an alog    " << endl;
    cout << "  file for importing into Matlab, Excel, or similar.  Age of each" << endl;
    cout << "  variable is also reported (may be negative).  Variables are    " << endl;
    cout << "  reported for each acomms transmission or reception.            " << endl;
    cout << "                                                                 " << endl;
    cout << "Standard Arguments:                                              " << endl;
    cout << "  in.alog  - The input logfile.                                  " << endl;
    cout << "  VAR      - The name of a MOOS variable                         " << endl;
    cout << "                                                                 " << endl;
    cout << "                                                                 " << endl;
    cout << "Options:                                                         " << endl;
    cout << "  -h,--help        Displays this help message                    " << endl;
    cout << "  -b,--backsearch  When printing a line, look only backards for  " << endl;
    cout << "                   the latest posting of each variable. (Age will" << endl;
    cout << "                   always be positive).                          " << endl;
    cout << "  --backsearch=X   Allow searching up to X seconds in the future " << endl;
    cout << "                   for the nearest posting of each variable.     " << endl;
    cout << "  --delimiter=\"X\"  Set the delimiter string to X.  Default is ,  " << endl;
    cout << "                                                                 " << endl;
    cout << "                                                                 " << endl;
    cout << "Further Notes:                                                   " << endl;
    cout << "  (1) The output file will have the same filename as the input,  " << endl;
    cout << "      but with \"receive\" or \"transmit\" and a .csv extension. " << endl;
    cout << "  (2) Use * for wildcard logging (e.g. VAR* matches any MOOS     " << endl;
    cout << "      variable starting with VAR)                                " << endl;
    cout << endl;
}


bool entry_sort( ALogEntry e1, ALogEntry e2 ) {
	return e1.getTimeStamp() < e2.getTimeStamp();
}

string printTransmit(ALogEntry entry) {
	string msg_val = entry.getStringVal();

	goby::acomms::protobuf::ModemTransmission trans;
	string temp;
	while ( msg_val.find("<|>") != string::npos ) {
		msg_val.replace( msg_val.find("<|>"), 3, "\n" );
	}
	string const* ptr = &msg_val;
	google::protobuf::TextFormat::ParseFromString( *ptr, &trans );

	stringstream ss;
	ss << trans.src() << delimiter;
	ss << trans.dest() << delimiter;
	ss << trans.rate() << delimiter;
	ss << trans.type() << delimiter;
	ss << trans.ack_requested() << delimiter;
	int i;
	for (i=0; i<trans.frame_size(); i++) {
		string frame = trans.frame(i);
		while (frame.find("\n",0) != string::npos) {
			frame.erase(frame.find("\n",0),1);
		}
		while (frame.find("\r",0) != string::npos) {
			frame.erase(frame.find("\r",0),1);
		}
		while (frame.find(delimiter,0) != string::npos) {
			frame.erase(frame.find(delimiter,0),delimiter.size());
		}
		ss << frame << delimiter;
	}
	while (i < 8) {
		i++;
		ss << delimiter;
	}
	if (trans.HasExtension(micromodem::protobuf::type)) {
		ss << trans.GetExtension(micromodem::protobuf::type);
	}

	return ss.str();
}

string printTransmitHeader() {
	stringstream ss;
	ss << "src" << delimiter;
	ss << "dest" << delimiter;
	ss << "rate" << delimiter;
	ss << "type" << delimiter;
	ss << "ack_requested" << delimiter;
	ss << "frame0" << delimiter;
	ss << "frame1" << delimiter;
	ss << "frame2" << delimiter;
	ss << "frame3" << delimiter;
	ss << "frame4" << delimiter;
	ss << "frame5" << delimiter;
	ss << "frame6" << delimiter;
	ss << "frame7" << delimiter;
	ss << "umodem_protobuf_type";

	return ss.str();
}

string printStats(micromodem::protobuf::ReceiveStatistics &stats) {
	stringstream ss;

	ss << stats.mode() << delimiter;
	ss << stats.time() << delimiter;
	ss << stats.clock_mode() << delimiter;
	ss << stats.mfd_peak() << delimiter;
	ss << stats.mfd_power() << delimiter;
	ss << stats.mfd_ratio() << delimiter;
	ss << stats.spl() << delimiter;
	ss << stats.shf_agn() << delimiter;
	ss << stats.shf_ainpshift() << delimiter;
	ss << stats.shf_ainshift() << delimiter;
	ss << stats.shf_mfdshift() << delimiter;
	ss << stats.shf_p2bshift() << delimiter;
	ss << stats.rate() << delimiter;
	ss << stats.source() << delimiter;
	ss << stats.dest() << delimiter;
	ss << stats.psk_error_code() << delimiter;
	ss << stats.packet_type() << delimiter;
	ss << stats.number_frames() << delimiter;
	ss << stats.number_bad_frames() << delimiter;
	ss << stats.snr_rss() << delimiter;
	ss << stats.snr_in() << delimiter;
	ss << stats.snr_out() << delimiter;
	ss << stats.snr_symbols() << delimiter;
	ss << stats.mse_equalizer() << delimiter;
	ss << stats.data_quality_factor() << delimiter;
	ss << stats.doppler() << delimiter;
	ss << stats.stddev_noise() << delimiter;
	ss << stats.carrier_freq() << delimiter;
	ss << stats.bandwidth() << delimiter;
	ss << stats.version();

	return ss.str();
}

string printStatHeader() {
	stringstream ss;

	ss << "mode" << delimiter;
	ss << "time" << delimiter;
	ss << "clock_mode" << delimiter;
	ss << "mfd_peak" << delimiter;
	ss << "mfd_power" << delimiter;
	ss << "mfd_ratio" << delimiter;
	ss << "spl" << delimiter;
	ss << "shf_agn" << delimiter;
	ss << "shf_ainpshift" << delimiter;
	ss << "shf_ainshift" << delimiter;
	ss << "shf_mfdshift" << delimiter;
	ss << "shf_p2bshift" << delimiter;
	ss << "rate" << delimiter;
	ss << "source" << delimiter;
	ss << "dest" << delimiter;
	ss << "psk_error_code" << delimiter;
	ss << "packet_type" << delimiter;
	ss << "number_frames" << delimiter;
	ss << "number_bad_frames" << delimiter;
	ss << "snr_rss" << delimiter;
	ss << "snr_in" << delimiter;
	ss << "snr_out" << delimiter;
	ss << "snr_symbols" << delimiter;
	ss << "mse_equalizer" << delimiter;
	ss << "data_quality_factor" << delimiter;
	ss << "doppler" << delimiter;
	ss << "stddev_noise" << delimiter;
	ss << "carrier_freq" << delimiter;
	ss << "bandwidth" << delimiter;
	ss << "version";

	return ss.str();
}

string printEmptyStats() {
	stringstream ss;
	for (int i=0; i<29; i++) {
		ss << delimiter;
	}
	return ss.str();
}

string printReceive(ALogEntry entry) {
	string msg_val = entry.getStringVal();

	goby::acomms::protobuf::ModemTransmission trans;
	string temp;
	while ( msg_val.find("<|>") != string::npos ) {
		msg_val.replace( msg_val.find("<|>"), 3, "\n" );
	}
	string const* ptr = &msg_val;
	google::protobuf::TextFormat::ParseFromString( *ptr, &trans );


	stringstream ss;

	ss << trans.src() << delimiter;
	ss << trans.dest() << delimiter;
	ss << trans.time() << delimiter;
	ss << trans.time_source() << delimiter;
	ss << trans.type() << delimiter;
	ss << trans.ack_requested() << delimiter;
	int i;
	for (i=0; i<trans.frame_size(); i++) {
		string frame = trans.frame(i);
		while (frame.find("\n",0) != string::npos) {
			frame.erase(frame.find("\n",0),1);
		}
		while (frame.find("\r",0) != string::npos) {
			frame.erase(frame.find("\r",0),1);
		}
		while (frame.find(delimiter,0) != string::npos) {
			frame.erase(frame.find(delimiter,0),delimiter.size());
		}
		ss << frame << delimiter;
	}
	while (i < 8) {
		i++;
		ss << delimiter;
	}
	if (trans.HasExtension(micromodem::protobuf::type)) {
		ss << trans.GetExtension(micromodem::protobuf::type);
	}
	ss << delimiter;
	if (trans.HasExtension(micromodem::protobuf::ranging_reply)) {
		micromodem::protobuf::RangingReply rr = trans.GetExtension(micromodem::protobuf::ranging_reply);
		for (i=0; i<rr.one_way_travel_time_size(); i++) {
			ss << rr.one_way_travel_time(i) << delimiter;
		}
		while (i<4) {
			i++;
			ss << delimiter;
		}
		ss << rr.ambiguity() << delimiter;
		ss << rr.is_one_way_synchronous() << delimiter;
		ss << rr.receiver_clk_mode() << delimiter;
	} else {
		for (i=0; i<7; i++) {
			ss << delimiter;
		}
	}

	micromodem::protobuf::ReceiveStatistics statistics;
	int numstats = trans.ExtensionSize( micromodem::protobuf::receive_stat );
	if (numstats == 1) {
		statistics = trans.GetExtension( micromodem::protobuf::receive_stat, 0 );
		ss << printStats(statistics) << delimiter;
	} else if (numstats == 2) {
		statistics = trans.GetExtension( micromodem::protobuf::receive_stat, 1 );
		ss << printStats(statistics) << delimiter;
	} else {
		ss << printEmptyStats() << delimiter;
	}

	for (int i=0; i<trans.ExtensionSize(micromodem::protobuf::frame_with_bad_crc); i++) {
		ss << trans.GetExtension(micromodem::protobuf::frame_with_bad_crc,i);
	}

	return ss.str();
}

string printReceiveHeader() {
	stringstream ss;
	ss << "src" << delimiter;
	ss << "dest" << delimiter;
	ss << "time" << delimiter;
	ss << "time_source" << delimiter;
	ss << "type" << delimiter;
	ss << "ack_requested" << delimiter;
	ss << "frame0" << delimiter;
	ss << "frame1" << delimiter;
	ss << "frame2" << delimiter;
	ss << "frame3" << delimiter;
	ss << "frame4" << delimiter;
	ss << "frame5" << delimiter;
	ss << "frame6" << delimiter;
	ss << "frame7" << delimiter;
	ss << "umodem_protobuf_type" << delimiter;
	ss << "one_way_travel_time0" << delimiter;
	ss << "one_way_travel_time1" << delimiter;
	ss << "one_way_travel_time2" << delimiter;
	ss << "one_way_travel_time3" << delimiter;
	ss << "ambiguity" << delimiter;
	ss << "is_one_way_synchronous" << delimiter;
	ss << "receiver_clk_mode" << delimiter;

	ss << printStatHeader() << delimiter;

	ss << "bad_frame_indices";

	return ss.str();
}

void printHeader() {
	output << "time";
	for ( int i=0; i<variables.size(); i++ ) {
		output << delimiter << variables[i] << delimiter << variables[i] << "_age";
	}
	if (mode == RECEIVE) {
		output << delimiter << printReceiveHeader();
	} else {
		output << delimiter << printTransmitHeader();
	}
	output << delimiter << hex_data_name;
	output << delimiter << hex_data_name << "_age";
	output << endl;
}

double backsearch_range = 0;

ALogEntry findNearestHex(double time) {
	double offset = 10000000;
	ALogEntry entry;
	for (int i=0; i<hex_data_entries.size(); i++) {
//	while (!hex_data_entries.empty()) {
		if (fabs(hex_data_entries[i].getTimeStamp()-time) < offset) {
			entry = hex_data_entries[i];
//			hex_data_entries.pop_front();
			offset = fabs(entry.getTimeStamp()-time);
		}// else
//			break;
	}
	return entry;
}

int main (	int argc, char *argv[] ) {
	string sync_variable;
	bool restrict_to_backsearch = false;

	if( scanArgs(argc, argv, "-h", "--help" ) ) {
		printHelp();
		return 0;
	}

	// read command line arguments
	string alogfile_in;
	for (int i = 1; i < argc; i++) {
		string sarg = argv[i];
		if (strContains(sarg, ".alog")) {
			if (alogfile_in == "")
				alogfile_in = sarg;
		} else if (sarg == "--receive") {
			mode = RECEIVE;
			sync_variable = "ACOMMS_RECEIVED_ALL";
			hex_data_name = "ACOMMS_RECEIVED_DATA_HEX";
		} else if (sarg == "--transmit") {
			mode = TRANSMIT;
			sync_variable = "ACOMMS_TRANSMITTED_ALL";
			hex_data_name = "ACOMMS_TRANSMITTED_DATA_HEX";
		} else if ( sarg == "-b" || sarg == "--backsearch" ) {
			restrict_to_backsearch = true;
		} else if ( strContains(sarg, "--backsearch=" ) ) {
		    restrict_to_backsearch = false;
		    backsearch_range = atof( readCommandArg(string(sarg)).c_str() );
		} else if ( strContains(sarg, "--delimiter=" ) ) {
			delimiter = readCommandArg(string(sarg));
		} else if ( sarg.find("*")!=string::npos ) {
			wilds.push_back( sarg );
		} else {
			variables.push_back( sarg );
		}
	}

	if (alogfile_in == "") {
		cout << "No alog file given - exiting" << endl;
		exit(0);
	}

	if (mode==UNSET) {
		cout << "Must specify --transmit or --recieve" << endl;
		exit(0);
	}

	FILE *logfile = fopen( alogfile_in.c_str(), "r" );
	if ( logfile == NULL ) {
		cout << "Alog file does not exist - exiting" << endl;
		exit(0);
	}

	vector<ALogEntry> entries;

	double start_time = -100;
	double stop_time = -100;
	// read through alog file, picking out variables we care about
	int line_num = 1;
	ALogEntry entry = getNextRawALogEntry_josh( logfile );
	while( entry.getStatus() != "eof" ) {
		line_num++;
		if ( entry.getStatus() == "okay" ) {
			string key = entry.getVarName();

			if (key == hex_data_name) {
				hex_data_entries.push_back(entry);
			}

			// save if its in list of variables
			if ( find(variables.begin(), variables.end(), key) != variables.end() ||
					key == sync_variable ) {
				entries.push_back( entry );
			} else {
				// if matches a wildcard, add to our variable list
				for ( int i=0; i<wilds.size(); i++ ) {
					if ( wildCardMatch(wilds[i], key) ) {
						variables.push_back(key);
						entries.push_back(entry);
						break;
					}
				}
			}
		}
		entry = getNextRawALogEntry_josh( logfile );
	}
	cout << "Read " << line_num << " lines from log file." << endl;

	// make sure entries are sorted by time
	sort( entries.begin(), entries.end(), entry_sort );
	cout << "Sorted " << entries.size() << " entries." << endl;

	sort( hex_data_entries.begin(), hex_data_entries.end(), entry_sort );

	// open output file and print header
	string file_out = alogfile_in;
	if (mode == RECEIVE) {
		file_out.replace( file_out.size()-5, 13, "_receive.csv");
	} else {
		file_out.replace( file_out.size()-5, 14, "_transmit.csv");
	}
	output.open(file_out.c_str());
	printHeader();

	if ( restrict_to_backsearch ) {
		// backsearching with sync variable
		// latest version of variables and their times
		map<string,string> current_value;
		map<string,double> current_times;
		for ( int i=0; i<variables.size(); i++ ) {
			current_value[variables[i]] = "-1";
			current_times[variables[i]] = -100000000;
		}
		for ( int i=0; i<entries.size(); i++ ) {
			ALogEntry entry = entries[i];
			string key = entry.getVarName();
			// update current values
			if ( find(variables.begin(), variables.end(), key) != variables.end() ) {
				current_value[key] = entry.getStringVal();
				current_times[key] = entry.getTimeStamp();
			}
			// output when key is found
			if ( key == sync_variable ) {
				output << entry.getTimeStamp();
				for ( int j=0; j<variables.size(); j++ ) {
					output << delimiter << current_value[variables[j]];
					output << delimiter << entry.getTimeStamp()-current_times[variables[j]];
				}
				if (mode == RECEIVE) {
					output << delimiter << printReceive(entry);
				} else {
					output << delimiter << printTransmit(entry);
				}
				ALogEntry hex_entry = findNearestHex(entry.getTimeStamp());
				output << delimiter << hex_entry.getStringVal() << delimiter << entry.getTimeStamp()-hex_entry.getTimeStamp();
				output << endl;
			}
		}
	} else {
		// full search with sync variable
		// latest version of variables and their times
		map<string,string> current_value;
		map<string,double> current_times;
		deque<PartialEntry> partials;
		for ( int i=0; i<variables.size(); i++ ) {
			current_value[variables[i]] = "-1";
			current_times[variables[i]] = -100000000;
		}
		for ( int i=0; i<entries.size(); i++ ) {
			ALogEntry entry = entries[i];
			string key = entry.getVarName();
			// update current values
			if ( find(variables.begin(), variables.end(), key) != variables.end() ) {
				current_value[key] = entry.getStringVal();
				current_times[key] = entry.getTimeStamp();
			}
			// start a partial on sync variable
			if ( key == sync_variable ) {
				partials.push_back( PartialEntry(
						variables, current_value, current_times, entry.getTimeStamp() ) );
				partials.back().acomms = entry;
			}

			// process current entry for all partials
			for ( int j=0; j<partials.size(); j++ ) {
                if ( backsearch_range == 0 ) {
					partials[j].process( key, entry.getStringVal(), entry.getTimeStamp() );
				} else {
					partials[j].process( key, entry.getStringVal(), entry.getTimeStamp(), backsearch_range );
				}
			}
			// output complete partials
			if ( !partials.empty() ) {
				while ( partials.front().checkComplete() ) {
					output << partials.front().serialize() << endl;
					partials.pop_front();
					if ( partials.empty() ) break;
				}
			}
		}
		// clear leftover partials
		while ( !partials.empty() ) {
			output << partials.front().serialize() << endl;
			partials.pop_front();
		}
	}

	output.close();

	return 0;
}

string readCommandArg( string sarg ) {
	int pos = sarg.find("=");
	if ( pos==string::npos || pos==sarg.size()-1 ) {
		return "";
	} else {
		return sarg.substr( pos+1, sarg.size()-pos+1 );
	}
}

// construct partial entry
PartialEntry::PartialEntry ( vector<string> vars,
		map<string,string> vals,
		map<string,double> times,
		double time ) {
	m_variables = vars;
	m_values = vals;
	m_times = times;
	m_time = time;
	for ( int i=0; i<vars.size(); i++ ) {
		m_verified[vars[i]]=false;
	}
}

// check if all variable verified
bool PartialEntry::checkComplete() {
	for ( int i=0; i<m_variables.size(); i++ ) {
		if ( !m_verified[m_variables[i]] ) return false;
	}
	return true;
}

// process a new entry
void PartialEntry::process( string var, string val, double time ) {
	// replace our value if this one is closer
	if ( fabs(time-m_time) < fabs(m_times[var]-m_time) ) {
		m_values[var]=val;
		m_times[var]=time;
	}
	// mark as verified
	m_verified[var]=true;
}

// process a new entry
void PartialEntry::process( string var, string val, double time, double max_forward ) {
    // replace our value if this one is closer
    if ( fabs(time-m_time) < fabs(m_times[var]-m_time) && time-m_time < max_forward ) {
        m_values[var]=val;
        m_times[var]=time;
    }
    // mark as verified
    m_verified[var]=true;
}

string PartialEntry::serialize() {
	// output to file
	stringstream ss;
	ss << m_time;
	for ( int i=0; i<m_variables.size(); i++ ) {
		ss << delimiter << m_values[m_variables[i]];
		ss << delimiter << m_time-m_times[m_variables[i]];
	}
	if (mode == RECEIVE) {
		ss << delimiter << printReceive(acomms);
	} else {
		ss << delimiter << printTransmit(acomms);
	}
	ALogEntry hex_entry = findNearestHex(acomms.getTimeStamp());
	ss << delimiter << hex_entry.getStringVal() << delimiter << acomms.getTimeStamp()-hex_entry.getTimeStamp();
	return ss.str();
}


// slightly modified version of that found in lib_logutils
// always returns a string variables and allows for spaces in variable value
ALogEntry getNextRawALogEntry_josh(FILE *fileptr, bool allstrings)
{
	ALogEntry entry;
	if (!fileptr) {
		cout << "failed getNextRawALogEntry() - null file pointer" << endl;
		entry.setStatus("invalid");
		return (entry);
	}

	bool EOLine = false;
	bool EOFile = false;
	int buffix = 0;
	int lineix = 0;
	int myint = '\0';
	char buff[MAX_LINE_LENGTH];

	string time, var, rawsrc, val;

	// Simple state machine:
	//   0: time
	//   1: between time and variable
	//   2: variable
	//   3: between variable and source
	//   4: source
	//   5: between source and value
	//   6: value
	int state = 0;

	while ((!EOLine) && (!EOFile) && (lineix < MAX_LINE_LENGTH)) {
		//		cout << "state: " << state << endl;
		myint = fgetc(fileptr);
		unsigned char mychar = myint;
		switch (myint) {
		case EOF:
			EOFile = true;
			break;
		case ' ':
			if (state == 6) {
				buff[buffix] = mychar;
				buffix++;
			}
			//			break;
		case '\t':
			if (state == 0) {
				buff[buffix] = '\0';
				time = buff;
				buffix = 0;
				state = 1;
			} else if (state == 2) {
				buff[buffix] = '\0';
				var = buff;
				buffix = 0;
				state = 3;
			} else if (state == 4) {
				buff[buffix] = '\0';
				rawsrc = buff;
				buffix = 0;
				state = 5;
			}
			break;
		case '\n':
			buff[buffix] = '\0'; // attach terminating NULL
			val = buff;
			EOLine = true;
			break;
		default:
			if (state == 1)
				state = 2;
			else if (state == 3)
				state = 4;
			else if (state == 5)
				state = 6;
			buff[buffix] = mychar;
			buffix++;
		}
		lineix++;
	}

	string src = biteString(rawsrc, ':');
	string srcaux = rawsrc;

	val = stripBlankEnds(val);

	//	cout << "t:" << time << " v:" << var << " s:" << src << " v:" << val << endl;

	if ((time != "") && (var != "") && (src != "") /*&& (val != "")*/
			&& isNumber(time)) {
		entry.set(atof(time.c_str()), var, src, srcaux, val);
		entry.setStatus("okay");
	} else {
		if (EOFile)
			entry.setStatus("eof");
		else
			entry.setStatus("invalid");
	}

	return (entry);
}

bool wildCardMatch( string wild, string key ) {
	if ( wild == key ) return true;
	if ( wild.empty() || key.empty() ) return false;
	if ( wild.find("*") == string::npos ) return false;

	int wild_position = wild.find("*");
	if ( wild_position == 0 ) {
		string post_wild = wild.substr(1, wild.size()-1);
		if ( key.size() < post_wild.size() ) return false;
		string key_end = key.substr( key.size()-post_wild.size(), post_wild.size() );
		if ( post_wild == key_end ) return true;
		else return false;
	} else if ( wild_position == wild.size()-1 ) {
		string pre_wild = wild.substr(0,wild_position);
		if ( key.size() < pre_wild.size() ) return false;
		string key_start = key.substr( 0, pre_wild.size() );
		if ( pre_wild == key_start ) return true;
		else return false;
	} else {
		string pre_wild = wild.substr(0,wild_position);
		string post_wild = wild.substr(wild_position+1, wild.size()-wild_position-1);
		if ( key.size() < pre_wild.size() + post_wild.size() ) return false;
		string key_begin = key.substr( 0, pre_wild.size() );
		string key_end = key.substr( key.size()-post_wild.size(), post_wild.size() );
		if ( key_begin==pre_wild && key_end==post_wild ) return true;
		else return false;
	}
}

// everything past here is identical to the moos-ivp libraries

string biteString(string& str, char separator)
{
  string::size_type len = str.length();
  if(len == 0)
    return("");

  bool found = false;
  string::size_type ix=0;
  string::size_type i=0;
  for(i=0; (!found && i<len); i++) {
    if(str[i] == separator) {
      found = true;
      ix = i;
    }
  }

  if(!found) {
    string str_front = str;
    str = "";
    return(str_front);
  }

  string str_front(str.c_str(), ix);
  string str_back;
  if((ix+1) < len)
    str_back = str.substr(ix+1);
  str = str_back;

  return(str_front);
}

string stripBlankEnds(const string& str)
{
  if(str.length() == 0)
    return("");

  const char *sPtr = str.c_str();
  int length = strlen(sPtr);

  int i;
  int startIX=length; // assume all blanks

#if 0
  // Added Dec 23rd 2007 (mikerb)
  // Quick check to see if the string has already been stripped
  // of blank ends. Just return the original in that case.
  int s_char = (int)(sPtr[0]);
  int e_char = (int)(sPtr[length-1]);
  if((s_char != 9) && (s_char != 32) &&
     (e_char != 9) && (e_char != 32))
    return(str);
#endif

  for(i=0; i<length; i++) {
    int cval = (int)(sPtr[i]);
    if(!((cval==9) ||     // 09:tab
	 (cval==32))) {   // 32:blank
      startIX = i;
      i = length;
    }
  }

  if(sPtr[startIX]==0)   // If first non-blank is NULL term,
    startIX=length;      // treat as if blank, empty line.


  if(startIX==length) {      // Handle case where the
    string ret_string = "";  // whole string was blanks
    return(ret_string);      // and/or tabs
  }

  int endIX=-1;

  for(i=length-1; i>=0; i--) {
    int cval = (int)(sPtr[i]);
    if(!((cval==9)  ||    // 09:tab
	 (cval==0)  ||    // 00:NULL terminator
	 (cval==32) ||    // 32:blank
	 (cval==13) ||    // 13:car-return
	 (cval==10))) {   // 10:newline
      endIX = i;
      i = -1;
    }
  }

  if(startIX > endIX) {      // Handle case where the
    string ret_string = "";  // whole string was blanks,
    return(ret_string);      // tabs, newlines, or car-return
  }

  length = (endIX - startIX) + 1;

  char *buff = new char [length+1];
  strncpy(buff, sPtr+startIX, length);
  buff[length] = '\0';

  string ret_string = buff;
  delete [] buff;
  return(ret_string);
}

bool isNumber(const string& str, bool blanks_allowed)
{
  string newstr = str;
  if(blanks_allowed)
    newstr = stripBlankEnds(str);

  if(newstr.length() == 0)
    return(false);

  if((newstr.length() > 1) && (newstr.at(0) == '+'))
    newstr = newstr.substr(1, newstr.length()-1);

  const char *buff = newstr.c_str();

  string::size_type  len = newstr.length();
  int  digi_cnt = 0;
  int  deci_cnt = 0;
  bool ok       = true;

  for(string::size_type i=0; (i<len)&&ok; i++) {
    if((buff[i] >= 48) && (buff[i] <= 57))
      digi_cnt++;
    else if(buff[i] == '.') {
      deci_cnt++;
      if(deci_cnt > 1)
	ok = false;
    }
    else if(buff[i] == '-') {
      if((digi_cnt > 0) || (deci_cnt > 0))
	ok = false;
    }
    else
      ok = false;
  }

  if(digi_cnt == 0)
    ok = false;

  return(ok);
}

bool scanArgs(int argc, char **argv, const char* str1,
	      const char *str2, const char *str3)
{
  for(int i=0; i<argc; i++) {
    bool match1 = !strncmp(str1, argv[i], strlen(argv[i]));
    bool match2 = !strncmp(str1, argv[i], strlen(str1));
    if(match1 && match2)
      return(true);

    if(str2) {
      match1 = !strncmp(str2, argv[i], strlen(argv[i]));
      match2 = !strncmp(str2, argv[i], strlen(str2));
      if(match1 && match2)
	return(true);
    }

    if(str3) {
      match1 = !strncmp(str3, argv[i], strlen(argv[i]));
      match2 = !strncmp(str3, argv[i], strlen(str2));
      if(match1 && match2)
	return(true);
    }
  }
  return(false);
}

bool strContains(const string& str, const string& qstr)
{
  string::size_type posn = str.find(qstr, 0);
  if(posn == string::npos)
    return(false);
  else
    return(true);
}
