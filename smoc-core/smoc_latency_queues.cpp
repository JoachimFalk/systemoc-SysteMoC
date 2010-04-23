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

#include <systemoc/smoc_config.h>

#include <systemoc/detail/smoc_latency_queues.hpp>

#include <systemoc/detail/hscd_tdsim_TraceLog.hpp>

#ifdef SYSTEMOC_ENABLE_VPC
# include <systemcvpc/hscd_vpc_Director.h>
#endif //SYSTEMOC_ENABLE_VPC

#ifdef SYSTEMOC_ENABLE_VPC
namespace Detail {

# ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
  struct DeferedTraceLogDumper
  : public smoc_event_listener,
    public SysteMoC::Detail::SimCTXBase {
    smoc_ref_event_p  event;
    smoc_root_chan   *fifo;
    const char       *mode;

    void signaled(smoc_event_waiter *_e) {
//    const char *name = fifo->name();
      
      this->getSimCTX()->getDataflowTraceLog()->traceStartActor(fifo, mode);
#   ifdef SYSTEMOC_DEBUG
      std::cerr << "smoc_detail::DeferedTraceLogDumper::signaled(...)" << std::endl;
#   endif // SYSTEMOC_DEBUG
      assert(_e == event.get());
      assert(*_e);
      //event = NULL;
      this->getSimCTX()->getDataflowTraceLog()->traceEndActor(fifo);
      return;
    }
    void eventDestroyed(smoc_event_waiter *_e) {
#   ifdef SYSTEMOC_DEBUG
      std::cerr << "smoc_detail::DeferedTraceLogDumper:: eventDestroyed(...)" << std::endl;
#   endif // SYSTEMOC_DEBUG
      delete this;
    }

    DeferedTraceLogDumper
      (const smoc_ref_event_p &event, smoc_root_chan *fifo, const char *mode)
      : event(event), fifo(fifo), mode(mode) {};
 
    virtual ~DeferedTraceLogDumper() {}
  };

# endif // SYSTEMOC_ENABLE_DATAFLOW_TRACE

  void LatencyQueue::actorTokenLatencyExpired(TokenInfo ti) {

    // TODO (ms): "unroll n"
    for(size_t n = ti.count; n > 0; --n) {
# ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
      this->getSimCTX()->getDataflowTraceLog()->traceStartActor(chan, "s");
//    this->getSimCTX()->getDataflowTraceLog()->traceStartFunction("transmit");
//    this->getSimCTX()->getDataflowTraceLog()->traceEndFunction("transmit");
      this->getSimCTX()->getDataflowTraceLog()->traceEndActor(chan);
# endif

#ifdef SYSTEMOC_DEBUG
      std::cerr << "VPC::write(" << ti.vpcIf.portIf->actor << ", "
                << ti.vpcIf.portIf->channel << ")" << std::endl;
#endif // SYSTEMOC_DEBUG
      // new FastLink interface
    //chan->vpcLink->compute(p);
      SystemC_VPC::EventPair events = ti.vpcIf.startWrite(1);

# ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
      if (!*events.dii) {
        // dii event not signaled
        //TODO (ms): remove reference to events.dii
        //           ref'counted events are support in VPC
        events.dii->addListener(new DeferedTraceLogDumper(events.dii, chan, "e"));
      } else {
        this->getSimCTX()->getDataflowTraceLog()->traceStartActor(chan, "e");
        this->getSimCTX()->getDataflowTraceLog()->traceEndActor(chan);
      }
      if (!*events.latency) {
        // latency event not signaled
        events.latency->addListener(new DeferedTraceLogDumper(events.latency, chan, "l"));
      } else {
        this->getSimCTX()->getDataflowTraceLog()->traceStartActor(chan, "l");
        this->getSimCTX()->getDataflowTraceLog()->traceEndActor(chan);
      }
# endif // SYSTEMOC_ENABLE_DATAFLOW_TRACE
      visibleQueue.addEntry(1, events.latency);
    }
  }

} // namespace Detail

#endif // SYSTEMOC_ENABLE_VPC
