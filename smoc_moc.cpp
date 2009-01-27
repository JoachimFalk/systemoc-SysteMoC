//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
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

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_graph_type.hpp>
#include <systemoc/smoc_sr_signal.hpp>
#include <systemoc/smoc_multicast_sr_signal.hpp>
#include <systemoc/smoc_ngx_sync.hpp>

#include <CoSupport/DataTypes/oneof.hpp>

#ifdef SYSTEMOC_DEBUG
# define DEBUG_CODE(code) code
#else
# define DEBUG_CODE(code) do {} while(0);
#endif

using namespace CoSupport;

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
  if(smoc_modes::dumpFileSMX && smoc_modes::dumpSMXWithSim)
    dump();
}

void smoc_scheduler_top::end_of_elaboration() {
  g->finalise();
  g->reset();
  if(smoc_modes::dumpFileSMX && !smoc_modes::dumpSMXWithSim) {
    dump();
    sc_core::sc_stop();
  }
}
  
void smoc_scheduler_top::dump() {

  // FIXME

  smoc_modes::PGWriter pgw(*smoc_modes::dumpFileSMX);
    
  pgw << "<?xml version=\"1.0\"?>" << std::endl;
  pgw << "<!DOCTYPE networkgraph SYSTEM \"networkgraph.dtd\">" << std::endl;
  pgw << "<networkgraph name=\"smoc_modes::dump\">" << std::endl;
  pgw.indentUp();
  g->assemble( pgw );
  pgw << "<architecturegraph name=\"architecture graph\" id=\"" << SysteMoC::NGXSync::idPool.printId() << "\">" << std::endl;
  pgw << "</architecturegraph>" << std::endl;
  pgw <<  "<mappings>" << std::endl;
  pgw <<  "</mappings>" << std::endl;
  pgw.indentDown();
  pgw << "</networkgraph>" << std::endl;
}

// FIXME: only needed in scheduleSR
// remove after redesign of SR scheduler
/*void smoc_scheduler_top::getLeafNodes(
    smoc_node_list &nodes, smoc_graph_base *node)
{
  const smoc_node_list& n = node->getNodes();
  
  for ( smoc_node_list::const_iterator iter = n.begin();
        iter != n.end();
        ++iter ) {
    if ( dynamic_cast<smoc_actor *>(*iter) != NULL ) {
      nodes.push_back(*iter);
    }
    if ( dynamic_cast<smoc_graph_base *>(*iter) != NULL ) {
      getLeafNodes(nodes, dynamic_cast<smoc_graph_base *>(*iter));
    }
  }
}*/

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

