//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2009 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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

#ifndef _INCLUDED_DETAIL_SMOC_LATENCY_QUEUES_HPP
#define _INCLUDED_DETAIL_SMOC_LATENCY_QUEUES_HPP

#include <list>

#include <boost/function.hpp>

#include <systemoc/smoc_config.h>

#include "smoc_event_decls.hpp"
#include "smoc_root_chan.hpp"

#include <smoc/smoc_simulation_ctx.hpp>

#ifdef SYSTEMOC_ENABLE_VPC
# include <vpc.hpp>

namespace Detail {

#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
struct DeferedTraceLogDumper;
#endif


class TokenInfo{
public:
  TokenInfo(size_t count, SysteMoC::Detail::VpcInterface vpcIf) :
    count(count), vpcIf(vpcIf) {}
  size_t                           count;
  SysteMoC::Detail::VpcInterface   vpcIf;
};

template<typename T>
class EventQueue
: protected smoc_event_listener {
private:
  typedef EventQueue this_type;
protected:
  typedef std::pair<T, smoc_ref_event_p>   Entry;
  typedef std::list<Entry>                 Queue;
protected:
  Queue                            queue;
  const boost::function<void (T)>  process;
protected:

  template<typename TT>
  void signaled_helper(std::list<std::pair<TT, smoc_ref_event_p> > &queue){
    do {
      process(queue.front().first);
      queue.pop_front();
    }
    while(!queue.empty() && *queue.front().second);
  }

  void signaled_helper(std::list<std::pair<size_t, smoc_ref_event_p> > &queue){
    size_t n = 0;

    do {
      n += queue.front().first;
      queue.pop_front();
    }
    while(!queue.empty() && *queue.front().second);
    
    process(n);
  }

  /// @brief See smoc_event_listener
  void signaled(smoc_event_waiter *e){
    assert(*e);
    assert(!queue.empty());
    assert(e == queue.front().second.get());
    
    e->delListener(this);
        
    this->signaled_helper(queue);

    if(!queue.empty())
      queue.front().second->addListener(this);    
  }

  void eventDestroyed(smoc_event_waiter *_e)
    { assert(!"eventDestroyed must never be called!"); }
public:
  EventQueue(const boost::function<void (T)> &proc)
    : process(proc) {}

  /// @brief Queue event  
  void addEntry(T t, const smoc_ref_event_p& le) {
    bool queueEmpty = queue.empty();
    
    if(queueEmpty && (!le || *le)) {
      // shortcut processing
      process(t);
    } else {
      queue.push_back(Entry(t, le));
      if(queueEmpty)
        le->addListener(this);
    }
  }

};


class LatencyQueue : public SysteMoC::Detail::SimCTXBase {
private:
  typedef LatencyQueue this_type;
protected:
  EventQueue<TokenInfo>    requestQueue;
  EventQueue<size_t>       visibleQueue;
  smoc_ref_event_p dummy;
  smoc_root_chan  *chan;
protected:
  /// @brief See EventQueue
  void actorTokenLatencyExpired(TokenInfo ti);
public:
  LatencyQueue(
      const boost::function<void (size_t)> &latencyExpired,
      smoc_root_chan *chan)
    : requestQueue(std::bind1st(
        std::mem_fun(&this_type::actorTokenLatencyExpired), this)),
      visibleQueue(latencyExpired), dummy(new smoc_ref_event()), chan(chan) {}

  void addEntry(size_t n, const smoc_ref_event_p &latEvent,
                SysteMoC::Detail::VpcInterface vpcIf)
    { requestQueue.addEntry(TokenInfo(n, vpcIf), latEvent); }
};

class DIIQueue {
private:
  typedef DIIQueue this_type;
protected:
  EventQueue<size_t> eventQueue;
public:
  DIIQueue(
      const boost::function<void (size_t)> &diiExpired)
    : eventQueue(diiExpired) {}

  void addEntry(size_t n, const smoc_ref_event_p &diiEvent,
                SysteMoC::Detail::VpcInterface vpcIf)
    { eventQueue.addEntry(n, diiEvent); }
};

} // namespace Detail
#endif // SYSTEMOC_ENABLE_VPC

#endif // _INCLUDED_DETAIL_SMOC_LATENCY_QUEUES_HPP
