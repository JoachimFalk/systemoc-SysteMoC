
/*******************************************************************************
 *                        Copyright 2005
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SysteMoC V2
 * Comment:
 * -----------------------------------------------------------------------------
 * hscd_tdsim_TraceLog.hpp
 * -----------------------------------------------------------------------------
 * Modifications History:
 * -----------------------------------------------------------------------------
 * Notes:
 * -----------------------------------------------------------------------------
 * $log$
 ******************************************************************************/

#ifndef __TRACELOG_H
#define __TRACELOG_H

#include <iostream>
#include <fstream>

#include <string>
#include <map>
#include <set>

using std::string;
class TraceLogStream {
private:
  std::ostream &stream;
  std::ofstream file;
  std::set<string> actors;
  std::map<string,std::set<string> > functions;
  std::map<string, int> function_call_count;
  std::map<string, int> actor_activation_count;
  std::string lastactor;

public:
  template <typename T>
  inline
  const TraceLogStream &operator << (const T &t) const {
    stream << "TraceLog: " << t << std::flush;
    return *this;
  }
  
  inline
  const TraceLogStream &operator << (std::ostream& (*manip)(std::ostream&)) const {
    stream << manip;
    return *this;
  }

  inline
  const TraceLogStream &operator << (std::ios_base& (*manip)(std::ios_base&)) const {
    stream  << manip;
    return *this;
  }


  TraceLogStream():stream(std::cerr){}

  TraceLogStream(const char * filename):stream(file){
    string fstring=filename;
    char *prefix=getenv("VPCTRACEFILEPREFIX");
    if( 0 != prefix ) fstring.insert(0,prefix);
    file.open(fstring.c_str());
  }

  ~TraceLogStream();
  
  void traceStartActor(const char * actor);
  void traceEndActor(const char * actor);
  void traceStartFunction(const char * actor);
  void traceEndFunction(const char * actor);
  void traceStartTryExecute(const char * actor);
  void traceEndTryExecute(const char * actor);
  void traceCommExecIn(size_t size, const char *name);
  void traceCommExecOut(size_t size, const char *name);
  

};

extern TraceLogStream TraceLog; 




#endif //__TRACELOG_H
