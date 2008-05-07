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

#include "smoc_event_decls.hpp"
#include <queue>

#ifdef SYSTEMOC_ENABLE_VPC
# include <systemcvpc/hscd_vpc_Director.h>
#endif //SYSTEMOC_ENABLE_VPC

#ifdef SYSTEMOC_ENABLE_VPC
namespace Detail {

# ifdef SYSTEMOC_TRACE
  struct DeferedTraceLogDumper;
# endif

  template<class FIFO_TYPE>
  class LatencyQueue {    
  public:
    typedef LatencyQueue<FIFO_TYPE> this_type;
  public:
    class RequestQueue
    : public smoc_event_listener {
    protected:
      typedef std::pair<size_t, smoc_ref_event_p> Entry;
      typedef std::queue<Entry>                   Queue;
    protected:
      Queue queue;

      smoc_event dummy;
      this_type* top;
    protected:
      LatencyQueue<FIFO_TYPE> &getTop()
        { return *top; }

      void doSomething(size_t n);

      void signaled(smoc_event_waiter *_e) {
        size_t n = 0;
        
        assert(*_e);
        assert(!queue.empty());
        assert(_e == &*queue.front().second);
        _e->delListener(this);
        do {
          n += queue.front().first;
          queue.pop(); // pop from front of queue
        } while (!queue.empty() && *queue.front().second);
        doSomething(n);
        if (!queue.empty())
          queue.front().second->addListener(this);
        return;// false;
      }

      void eventDestroyed(smoc_event_waiter *_e)
        { assert(!"eventDestroyed must never be called !!!"); }
    public:
      RequestQueue(LatencyQueue<FIFO_TYPE>* top) : top(top) {}
      void addEntry(size_t n, const smoc_ref_event_p &le) {
        bool queueEmpty = queue.empty();
        
        if (queueEmpty && (!le || *le)) {
          doSomething(n);
        } else {
          queue.push(Entry(n, le)); // insert at back of queue
          if (queueEmpty)
            le->addListener(this);
        }
      }

      virtual ~RequestQueue() {}
    } requestQueue;

    class VisibleQueue
    : public smoc_event_listener {
    protected:
      typedef std::pair<size_t, smoc_ref_event_p> Entry;
      typedef std::queue<Entry>                   Queue;
    protected:
      Queue queue;
      LatencyQueue<FIFO_TYPE>* top;
    protected:
       LatencyQueue<FIFO_TYPE> &getTop()
        { return *top; }

      void doSomething(size_t n);

      void signaled(smoc_event_waiter *_e) {
        size_t n = 0;
        
        assert(*_e);
        assert(!queue.empty());
        assert(_e == &*queue.front().second);
        _e->delListener(this);
        do {
          n += queue.front().first;
          queue.pop(); // pop from front of queue
        } while (!queue.empty() && *queue.front().second);
        doSomething(n);
        if (!queue.empty())
          queue.front().second->addListener(this);
        return;// false;
      }

      void eventDestroyed(smoc_event_waiter *_e)
        { assert(!"eventDestroyed must never be called !!!"); }
    public:
      VisibleQueue(LatencyQueue<FIFO_TYPE>* top) : top(top) {}
      void addEntry(size_t n, const smoc_ref_event_p &le) {
        bool queueEmpty = queue.empty();
        
        if (queueEmpty && (!le || *le)) {
          doSomething(n);
        } else {
          queue.push(Entry(n, le)); // insert at back of queue
          if (queueEmpty)
            le->addListener(this);
        }
      }

      virtual ~VisibleQueue() {}
    } visibleQueue;

  protected:

    FIFO_TYPE *fifo;
  public:
     LatencyQueue<FIFO_TYPE>(FIFO_TYPE *fifo)
      : requestQueue(this), visibleQueue(this), fifo(fifo) {}
    void addEntry(size_t n, const smoc_ref_event_p &le)
      { requestQueue.addEntry(n, le); }
  };

  template<typename FIFO_TYPE>
  void LatencyQueue<FIFO_TYPE>::RequestQueue::doSomething(size_t n) {
//# ifdef SYSTEMOC_TRACE
//    const char *name = getTop().fifo->name();
//# endif
    
    for (;n > 0; --n) {
      smoc_ref_event_p latEvent(new smoc_ref_event());
# ifdef SYSTEMOC_TRACE
      smoc_ref_event_p diiEvent(new smoc_ref_event());
      
      TraceLog.traceStartActor(getTop().fifo, "s");
//    TraceLog.traceStartFunction("transmit");
//    TraceLog.traceEndFunction("transmit");
      TraceLog.traceEndActor(getTop().fifo);
      
      SystemC_VPC::EventPair p(diiEvent.get(), latEvent.get());
# else
      SystemC_VPC::EventPair p(&dummy, latEvent.get());
# endif
      // new FastLink interface
      getTop().fifo->vpcLink->compute(p);
# ifdef SYSTEMOC_TRACE
      if (!*diiEvent) {
        // dii event not signaled
        diiEvent->addListener(new DeferedTraceLogDumper(diiEvent, getTop().fifo, "e"));
      } else {
        TraceLog.traceStartActor(getTop().fifo, "e");
        TraceLog.traceEndActor(getTop().fifo);
      }
      if (!*latEvent) {
        // latency event not signaled
        latEvent->addListener(new DeferedTraceLogDumper(latEvent, getTop().fifo, "l"));
      } else {
        TraceLog.traceStartActor(getTop().fifo, "l");
        TraceLog.traceEndActor(getTop().fifo);
      }
# endif
      getTop().visibleQueue.addEntry(1, latEvent);
    }
  }

  template<typename FIFO_TYPE>
  void LatencyQueue<FIFO_TYPE>::VisibleQueue::doSomething(size_t n) {
    getTop().fifo->latencyExpired(n);
  }

} // namespace Detail
#endif // SYSTEMOC_ENABLE_VPC

#endif // _INCLUDED_SMOC_LATENCY_QUEUES_HPP
