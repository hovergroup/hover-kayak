/*
 * acommsParse
 *        File: acommsParse.h
 *  Created on: May 23, 2013
 *      Author: Josh Leighton
 */

#ifndef ACOMMSPARSE_H_
#define ACOMMSPARSE_H_


#include <string>
#include <sstream>
#include <vector>
#include <map>


class ALogEntry
{
public:
  ALogEntry() {m_timestamp=0; m_dval=0; m_isnum=false;};
  ~ALogEntry() {};

  void set(double timestamp, const std::string& varname,
	   const std::string& source,
	   const std::string& srcaux,
	   const std::string& sval)
  {
    m_timestamp = timestamp;
    m_varname   = varname;
    m_source    = source;
    m_srcaux    = srcaux;
    m_sval      = sval;
    m_dval      = 0;
    m_isnum     = false;
  };

  void set(double timestamp, const std::string& varname,
	   const std::string& source,
	   const std::string& srcaux,
	   double dval)
  {
    m_timestamp = timestamp;
    m_varname   = varname;
    m_source    = source;
    m_srcaux    = srcaux;
    m_sval      = "";
    m_dval      = dval;
    m_isnum     = true;
  };

  void setStatus(const std::string& s) {m_status=s;};

  double      time() const         {return(m_timestamp);};
  double      getTimeStamp() const {return(m_timestamp);};
  std::string getVarName() const   {return(m_varname);};
  std::string getSource() const    {return(m_source);};
  std::string getSrcAux() const    {return(m_srcaux);};
  std::string getStringVal() const {return(m_sval);};
  double      getDoubleVal() const {return(m_dval);};
  bool        isNumerical() const  {return(m_isnum);};
  std::string getStatus() const    {return(m_status);};

  bool        isNull() const       {return(m_status=="null");};

  void        skewBackward(double v) {m_timestamp -= v;};
  void        skewForward(double v)  {m_timestamp += v;};

protected:
  double      m_timestamp;
  std::string m_varname;
  std::string m_source;
  std::string m_srcaux;
  std::string m_sval;
  double      m_dval;
  bool        m_isnum;

  // An optional status string. The empty string indicates the entry
  // is a normal entry. "invalid" means the entry is not normal. "eof"
  // could indicate that a the entry is the tail of normal entries.
  std::string  m_status;
};


// a class for storing entries when doing full searches
class PartialEntry {
public:
	PartialEntry( 	std::vector<std::string> vars,
					std::map<std::string,std::string> vals,
					std::map<std::string,double> times,
					double time );
	bool checkComplete();
	void process( std::string var, std::string val, double time );
	void process( std::string var, std::string val, double time, double max_forward );
	std::string serialize();
	ALogEntry acomms;

private:
	double m_time; // time stamp of the entry

	std::map<std::string,std::string> m_values;	// values of each variable
	std::map<std::string,double> m_times; // time stamp of each variable
	std::map<std::string,bool> m_verified;
	std::vector<std::string> m_variables;
};



#endif /* ACOMMSPARSE_H_ */
