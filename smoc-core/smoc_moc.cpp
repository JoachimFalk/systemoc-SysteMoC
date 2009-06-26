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

#include <systemoc/smoc_config.h>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_graph_type.hpp>
#include <systemoc/smoc_sr_signal.hpp>
#include <systemoc/smoc_multicast_sr_signal.hpp>
#include <smoc/smoc_simulation_ctx.hpp>

#include "SMXDumper.hpp"

namespace SGX = SystemCoDesigner::SGX;

smoc_scheduler_top::smoc_scheduler_top(smoc_graph_base* g) :
  sc_module(sc_module_name("smoc_scheduler_top")),
  g(g),
  simulation_running(false)
{
  SC_THREAD(schedule);
}

smoc_scheduler_top::smoc_scheduler_top(smoc_graph_base& g) :
  sc_module(sc_module_name("smoc_scheduler_top")),
  g(&g),
  simulation_running(false)
{
  SC_THREAD(schedule);
}

smoc_scheduler_top::~smoc_scheduler_top() {
  if(simulation_running)
    sc_core::sc_stop();
}

void smoc_scheduler_top::start_of_simulation()
{ simulation_running = true; }

void smoc_scheduler_top::end_of_simulation() {
  simulation_running = false;
#ifdef SYSTEMOC_ENABLE_SGX
  if (getSimCTX()->isSMXDumpingPostSimEnabled()) {
    SysteMoC::Detail::dumpSMX(getSimCTX()->getSMXPostSimFile(), getSimCTX(), *g);
  }
#endif // SYSTEMOC_ENABLE_SGX
}

void smoc_scheduler_top::end_of_elaboration() {
  g->finalise();
  g->reset();
#ifdef SYSTEMOC_ENABLE_SGX
  if (getSimCTX()->isSMXDumpingPreSimEnabled()) {
    SysteMoC::Detail::dumpSMX(getSimCTX()->getSMXPreSimFile(), getSimCTX(), *g);
    if (!getSimCTX()->isSMXDumpingPreSimKeepGoing())
      sc_core::sc_stop();
  }
#endif // SYSTEMOC_ENABLE_SGX
}
  
void smoc_scheduler_top::schedule() {
  smoc_transition_ready_list ol;
  
  // add outgoing transitions to list
  g->addCurOutTransitions(ol);
  
  while(true) {
    smoc_wait(ol);
    while(ol) {
#ifdef SYSTEMOC_DEBUG
      std::cerr << ol << std::endl;
#endif
      RuntimeTransition &transition = ol.getEventTrigger();
      // We have waited on a transition so it should no longer be blocked
      assert(transition);
      // It should either be enabled so we can execute it or its functionallity
      // condition could disable it.
      Expr::Detail::ActivationStatus status = transition.getStatus();
      
      switch(status.toSymbol()) {
        case Expr::Detail::_DISABLED:
          // remove disabled transition
          assert(&transition.getActor() == g);
          ol.remove(transition);
          break;
        case Expr::Detail::_ENABLED:
          // execute enabled transition
          assert(&transition.getActor() == g);
#ifdef SYSTEMOC_DEBUG
          std::cerr << "<node name=\"" << g->name() << "\">" << std::endl;
#endif
          // remove transitions from list
          g->delCurOutTransitions(ol);
          // execute transition
          transition.execute();
          // add transitions to list
          g->addCurOutTransitions(ol);
#ifdef SYSTEMOC_DEBUG
          std::cerr << "</node>" << std::endl;
#endif
          break;
        default:
          assert(!"WTF?! transition not either enabled or disabled!");
      }
    }
  }
}

