//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2008 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
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

#ifndef _INCLUDED_SMOC_LATENCY_QUEUES_HPP
#define _INCLUDED_SMOC_LATENCY_QUEUES_HPP

#include <list>
#include <systemoc/smoc_config.h>
#include "smoc_event_decls.hpp"
#include "smoc_root_chan.hpp"

#ifdef SYSTEMOC_ENABLE_VPC
# include <systemcvpc/hscd_vpc_Director.h>
#endif //SYSTEMOC_ENABLE_VPC

#include <boost/function.hpp>

#ifdef SYSTEMOC_ENABLE_VPC
namespace Detail {

#ifdef SYSTEMOC_TRACE
struct DeferedTraceLogDumper;
#endif

class EventQueue
: protected smoc_event_listener {
private:
  typedef EventQueue this_type;
protected:
  typedef std::pair<size_t, smoc_ref_event_p> Entry;
  typedef std::list<Entry>                    Queue;
protected:
  Queue                                 queue;
  const boost::function<void (size_t)>  process;
protected:
  /// @brief See smoc_event_listener
  void signaled(smoc_event_waiter *e);

  void eventDestroyed(smoc_event_waiter *_e)
    { assert(!"eventDestroyed must never be called!"); }
public:
  EventQueue(const boost::function<void (size_t)> &proc)
    : process(proc) {}

  /// @brief Queue event  
  void addEntry(size_t n, const smoc_ref_event_p& le);

};

class LatencyQueue {
private:
  typedef LatencyQueue this_type;
public:

  class ILatencyExpired {
  public:
    /// @brief Must call latencyExpired
    friend class LatencyQueue;
    /// @brief Virtual destructor
    virtual ~ILatencyExpired() {}
  protected:
    /// @brief Called when latency expired
    virtual void latencyExpired(size_t n) = 0;
  };

protected:
  EventQueue       requestQueue;
  EventQueue       visibleQueue;
  smoc_event       dummy;
  smoc_root_chan  *chan;
protected:
  /// @brief See EventQueue
  void actorTokenLatencyExpired(size_t n);
public:
  LatencyQueue(
      const boost::function<void (size_t)> &latencyExpired,
      smoc_root_chan *chan)
    : requestQueue(std::bind1st(
        std::mem_fun(&this_type::actorTokenLatencyExpired), this)),
      visibleQueue(latencyExpired),
      chan(chan) {}

  template <class T>
  LatencyQueue(T *chan)
    : requestQueue(std::bind1st(
        std::mem_fun(&this_type::actorTokenLatencyExpired), this)),
      visibleQueue(std::bind1st(
        std::mem_fun(&ILatencyExpired::latencyExpired), chan)),
      chan(chan) {}

  void addEntry(size_t n, const smoc_ref_event_p &latEvent)
    { requestQueue.addEntry(n, latEvent); }
};

class DIIQueue {
private:
  typedef DIIQueue this_type;
public:

  class IDIIExpired {
  public:
    /// @brief Must call diiExpired
    friend class DIIQueue;
    /// @brief Virtual destructor
    virtual ~IDIIExpired() {}
  protected:
    /// @brief Called when dii expired
    virtual void diiExpired(size_t n) = 0;
  };

protected:
  EventQueue       eventQueue;
  smoc_root_chan  *chan;
public:
  DIIQueue(
      const boost::function<void (size_t)> &diiExpired,
      smoc_root_chan *chan)
    : eventQueue(diiExpired),
      chan(chan) {}

  template <class T>
  DIIQueue(T *chan)
    : eventQueue(std::bind1st(
        std::mem_fun(&IDIIExpired::diiExpired), chan)),
      chan(chan) {}

  void addEntry(size_t n, const smoc_ref_event_p &diiEvent)
    { eventQueue.addEntry(n, diiEvent); }
};

} // namespace Detail
#endif // SYSTEMOC_ENABLE_VPC

#endif // _INCLUDED_SMOC_LATENCY_QUEUES_HPP
