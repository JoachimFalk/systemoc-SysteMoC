// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2017 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 * 
 * --- This software and any associated documentation is provided "as is" 
 * 
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifndef _INCLUDED_SMOC_DETAIL_TRACELOG_HPP
#define _INCLUDED_SMOC_DETAIL_TRACELOG_HPP

#include <iostream>
#include <fstream>

#include <string>
#include <map>
#include <set>

#include <systemoc/smoc_config.h>
#include "NamedIdedObj.hpp"
#include "SimCTXBase.hpp"
#include "../smoc_event.hpp"

#include <CoSupport/Streams/FilterOStream.hpp>
#include <CoSupport/Streams/IndentStreambuf.hpp>

#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE

# if !defined(SYSTEMOC_ENABLE_DEBUG)
#   error "SYSTEMOC_ENABLE_DATAFLOW_TRACE and !SYSTEMOC_ENABLE_DEBUG are incompatible !!!"
# endif

# if !defined(SYSTEMOC_NEED_IDS)
#   error "SYSTEMOC_ENABLE_DATAFLOW_TRACE and !SYSTEMOC_NEED_IDS are incompatible !!!"
# endif

class smoc_root_node;
class smoc_root_chan;
class smoc_func_call;

namespace smoc { namespace Detail {

class NamePool{
public:
  typedef std::map<size_t, std::string> NameMap;

  /**
   *
   */
  size_t registerId(std::string name, size_t id) {
    assert(names.find(id) == names.end() || names[id] == name);
    names[id] = name;
    return id;
  }
  
  /**
   *
   */
  const NameMap& getMap(){
    return names;
  }

private:
  NameMap names;
};

// dynamically obtained actor infos
struct s_actor_info {
  // actor's name
  std::string name;
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

class TraceLogStream : public SimCTXBase {
private:
  CoSupport::Streams::IndentStreambuf        indenter;
  mutable CoSupport::Streams::FilterOStream stream;
  std::ofstream file;
  std::set<std::string> actors;
  std::map<std::string,std::set<std::string> > functions;
  std::map<std::string, int> function_call_count;
  std::map<std::string, int> actor_activation_count;
  std::map<std::string, std::string> last_actor_function;
  std::string lastactor; 
  std::map<std::string, s_fifo_info> fifo_info;
  NamePool namePool;
public:
  template <typename T>
  inline
  const TraceLogStream &operator << (const T &t) const {

    if( !getSimCTX()->isDataflowTracingEnabled() )
      return *this;

    //stream << "TraceLog: " << t << std::flush;
    stream << t << std::flush;
    return *this;
  }

  inline
  const TraceLogStream &operator << (std::ostream& (*manip)(std::ostream&)) const {

    if( !getSimCTX()->isDataflowTracingEnabled() )
      return *this;

    stream << manip;
    return *this;
  }

  inline
  const TraceLogStream &operator << (std::ios_base& (*manip)(std::ios_base&)) const {

    if( !getSimCTX()->isDataflowTracingEnabled() )
      return *this;

    stream  << manip;
    return *this;
  }

  TraceLogStream(std::ostream *stream);

  void init();

  void traceStartActor(const smoc::Detail::NamedIdedObj *actor, const char *mode = "???");
  void traceEndActor(const smoc::Detail::NamedIdedObj *actor);
  void traceStartFunction(const smoc_func_call *func);
  void traceEndFunction(const smoc_func_call *func);
  void traceCommExecIn(const smoc_root_chan *chan, size_t size);
  void traceCommExecOut(const smoc_root_chan *chan, size_t size);
  void traceCommSetup(const smoc_root_chan *chan, size_t req);
  void traceTransition(size_t id);
  void traceInitialTokens(const smoc_root_chan *chan, size_t size, size_t capacity);

  void createFifoGraph();

  ~TraceLogStream();
private:
  TraceLogStream(const TraceLogStream & toCopy) :stream(std::cerr) {};
};

# ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
  struct DeferedTraceLogDumper
  : public smoc_event_listener,
    public smoc::Detail::SimCTXBase {
    smoc::Detail::NamedIdedObj   *fifo;
    const char       *mode;

    void signaled(smoc_event_waiter *_e) {
      assert(*_e);

#   ifdef SYSTEMOC_ENABLE_DEBUG
      std::cerr << "smoc_detail::DeferedTraceLogDumper::signaled(...)" << std::endl;
#   endif // SYSTEMOC_ENABLE_DEBUG
      this->getSimCTX()->getDataflowTraceLog()->traceStartActor(fifo, mode);
      this->getSimCTX()->getDataflowTraceLog()->traceEndActor(fifo);
      return;
    }
    void eventDestroyed(smoc_event_waiter *_e) {
#   ifdef SYSTEMOC_ENABLE_DEBUG
      std::cerr << "smoc_detail::DeferedTraceLogDumper:: eventDestroyed(...)" << std::endl;
#   endif // SYSTEMOC_ENABLE_DEBUG
      delete this;
    }

    DeferedTraceLogDumper
      (smoc::Detail::NamedIdedObj *obj, const char *mode)
      : fifo(obj), mode(mode) {};

    virtual ~DeferedTraceLogDumper() {}
  };

# endif // SYSTEMOC_ENABLE_DATAFLOW_TRACE

}} // namespace smoc::Detail

#endif

#endif /* _INCLUDED_SMOC_DETAIL_TRACELOG_HPP */
