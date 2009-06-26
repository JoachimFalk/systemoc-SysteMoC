// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2006 Hardware-Software-CoDesign, University of
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

#include <systemoc/smoc_config.h>

#include <systemoc/detail/smoc_latency_queues.hpp>

#ifdef SYSTEMOC_ENABLE_VPC
# include <systemcvpc/hscd_vpc_Director.h>
#endif //SYSTEMOC_ENABLE_VPC

#ifdef SYSTEMOC_ENABLE_VPC
namespace Detail {

# ifdef SYSTEMOC_TRACE
  struct DeferedTraceLogDumper
  : public smoc_event_listener {
    smoc_ref_event_p  event;
    smoc_fifo_kind   *fifo;
    const char       *mode;

    void signaled(smoc_event_waiter *_e) {
//    const char *name = fifo->name();
      
      TraceLog.traceStartActor(fifo, mode);
#   ifdef SYSTEMOC_DEBUG
      std::cerr << "smoc_detail::DeferedTraceLogDumper::signaled(...)" << std::endl;
#   endif // SYSTEMOC_DEBUG
      assert(_e == event.get());
      assert(*_e);
      event = NULL;
      TraceLog.traceEndActor(fifo);
      return;
    }
    void eventDestroyed(smoc_event_waiter *_e) {
#   ifdef SYSTEMOC_DEBUG
      std::cerr << "smoc_detail::DeferedTraceLogDumper:: eventDestroyed(...)" << std::endl;
#   endif // SYSTEMOC_DEBUG
      delete this;
    }

    DeferedTraceLogDumper
      (const smoc_ref_event_p &event, smoc_fifo_kind *fifo, const char *mode)
      : event(event), fifo(fifo), mode(mode) {};
 
    virtual ~DeferedTraceLogDumper() {}
  };

# endif // SYSTEMOC_TRACE

  void EventQueue::signaled(smoc_event_waiter *e) {
    size_t n = 0;
    
    assert(*e);
    assert(!queue.empty());
    assert(e == queue.front().second.get());
    
    e->delListener(this);
        
    do {
      n += queue.front().first;
      queue.pop_front();
    }
    while(!queue.empty() && *queue.front().second);
    
    process(n);
    
    if(!queue.empty())
      queue.front().second->addListener(this);    
  }

  void EventQueue::addEntry(size_t n, const smoc_ref_event_p& le) {
    bool queueEmpty = queue.empty();
    
    if(queueEmpty && (!le || *le)) {
      // shortcut processing
      process(n);
    } else {
      queue.push_back(Entry(n, le));
      if(queueEmpty)
        le->addListener(this);
    }
  }

  void LatencyQueue::actorTokenLatencyExpired(size_t n) {
    for(; n > 0; --n) {
      smoc_ref_event_p latEvent(new smoc_ref_event());
# ifdef SYSTEMOC_TRACE
      smoc_ref_event_p diiEvent(new smoc_ref_event());
      
      TraceLog.traceStartActor(chan, "s");
//    TraceLog.traceStartFunction("transmit");
//    TraceLog.traceEndFunction("transmit");
      TraceLog.traceEndActor(chan);
      
      SystemC_VPC::EventPair p(diiEvent.get(), latEvent.get());
# else
      SystemC_VPC::EventPair p(&dummy, latEvent.get());
# endif
      // new FastLink interface
      chan->vpcLink->compute(p);
# ifdef SYSTEMOC_TRACE
      if (!*diiEvent) {
        // dii event not signaled
        diiEvent->addListener(new DeferedTraceLogDumper(diiEvent, chan, "e"));
      } else {
        TraceLog.traceStartActor(chan, "e");
        TraceLog.traceEndActor(chan);
      }
      if (!*latEvent) {
        // latency event not signaled
        latEvent->addListener(new DeferedTraceLogDumper(latEvent, chan, "l"));
      } else {
        TraceLog.traceStartActor(chan, "l");
        TraceLog.traceEndActor(chan);
      }
# endif
      visibleQueue.addEntry(1, latEvent);
    }
  }

} // namespace Detail

#endif // SYSTEMOC_ENABLE_VPC
