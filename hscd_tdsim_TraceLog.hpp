// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2006 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef __TRACELOG_H
#define __TRACELOG_H

#include <iostream>
#include <fstream>

#include <string>
#include <map>
#include <set>

#ifdef SYSTEMOC_TRACE

using std::string;

// dynamically obtained actor infos
struct s_actor_info {
  // actor's name
  string name;
  // if name could be dynamically resolved
  bool unknown;
  // constructs unknown actor
  s_actor_info() :
    unknown(true)
  {}
};

// dynamically obtained fifo infos
struct s_fifo_info {
  // connected actors
  s_actor_info from;
  s_actor_info to;
  // currently contained messages
  int size;
  // constructs empty fifo
  s_fifo_info() :
    size(0)
  {}
};

// class for generating caption indices for fifos
class Sequence
{
public:
  // constructs object for generating "count" indices,
  // with each place ranging from "start" to "end"
  Sequence(unsigned int count, char start, char end);
  // returns current sequence
  const string& current() const;
  // resets to starting sequence
  void reset();
  // advances to next sequence, returns true if successful
  bool next();
private:
  unsigned int cnt;
  unsigned int gen;
  char start;
  char end;
  string seq;         
};

class TraceLogStream {
private:
  std::ostream &stream;
  std::ofstream file;
  std::set<string> actors;
  std::map<string,std::set<string> > functions;
  std::map<string, int> function_call_count;
  std::map<string, int> actor_activation_count;
  std::map<string, string> last_actor_function;
  std::string lastactor; 
  std::string fifo_actor_last;
  std::map<string, s_fifo_info> fifo_info;
  
public:
  template <typename T>
  inline
  const TraceLogStream &operator << (const T &t) const {
    //stream << "TraceLog: " << t << std::flush;
    stream << t << std::flush;
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
    stream << "<?xml version=\"1.0\"?>\n<systemoc_trace>" << std::endl;
  }

  ~TraceLogStream();
  
  void traceStartActor(const char * actor);
  void traceEndActor(const char * actor);
  void traceStartFunction(const char * func);
  void traceEndFunction(const char * func);
  void traceStartTryExecute(const char * actor);
  void traceEndTryExecute(const char * actor);
  void traceCommExecIn(size_t size, const char * actor);
  void traceCommExecOut(size_t size, const char * actor);
  void traceStartDeferredCommunication(const char * actor);
  void traceEndDeferredCommunication(const char * actor);
  void traceBlockingWaitStart();
  void traceBlockingWaitEnd();
  void traceStartChoice(const char * actor);
  void traceEndChoice(const char * actor);
  void traceStartTransact(const char * actor);
  void traceEndTransact(const char * actor);
  
  void createFifoGraph();
};

extern TraceLogStream TraceLog; 

#endif




#endif //__TRACELOG_H
