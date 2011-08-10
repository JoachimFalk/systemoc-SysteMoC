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

#include <smoc/detail/TraceLog.hpp>

#ifdef SYSTEMOC_ENABLE_VPC
# include <vpc.hpp>
#endif //SYSTEMOC_ENABLE_VPC

#ifdef SYSTEMOC_ENABLE_VPC
namespace Detail {
  void dump_helper(std::pair<size_t, smoc_vpc_event_p> & e){
    std::cerr << e.first << "\t" << e.second << "\t" << *e.second << std::endl;
  }
  void dump_helper(std::pair<TokenInfo, smoc_vpc_event_p> & e){
    std::cerr << e.first.count << "\t" << e.second << "\t" << *e.second << std::endl;
  }
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
        events.dii->addListener(new SysteMoC::Detail::DeferedTraceLogDumper(chan, "e"));
      } else {
        this->getSimCTX()->getDataflowTraceLog()->traceStartActor(chan, "e");
        this->getSimCTX()->getDataflowTraceLog()->traceEndActor(chan);
      }
      if (!*events.latency) {
        // latency event not signaled
        events.latency->addListener(new SysteMoC::Detail::DeferedTraceLogDumper(chan, "l"));
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
