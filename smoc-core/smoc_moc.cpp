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
#include <systemoc/detail/smoc_debug_stream.hpp>
#include <smoc/smoc_simulation_ctx.hpp>

#include <systemcvpc/Director.hpp>

#include "SMXDumper.hpp"


bool SysteMoC::Scheduling::canExecute(SystemC_VPC::ScheduledTask* actor) {
  assert(dynamic_cast<smoc_actor*>(actor) != NULL);
  return static_cast<smoc_actor*>(actor)->canFire();
}
void SysteMoC::Scheduling::execute(SystemC_VPC::ScheduledTask* actor) {
  //std::cerr << "SysteMoC::Scheduling::execute" << std::endl;
  assert(dynamic_cast<smoc_actor*>(actor) != NULL);
  static_cast<smoc_actor*>(actor)->schedule();
}

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
  boost::function<void (SystemC_VPC::ScheduledTask* actor)> callExecute
    = &SysteMoC::Scheduling::execute;
  boost::function<bool (SystemC_VPC::ScheduledTask* actor)> callCanExecute
    = &SysteMoC::Scheduling::canExecute;
  SystemC_VPC::Director::getInstance().
    registerSysteMoCCallBacks(callExecute, callCanExecute);

  g->finalise();

#ifdef SYSTEMOC_ENABLE_VPC
  //another finalise to patch the vpcCommTask
  // requires: ports finalised (in root_node::finalise)
  //           channel names (in root_chan::finalise)
  g->finaliseVpcLink();
  SystemC_VPC::Director::getInstance().endOfVpcFinalize();
#endif //SYSTEMOC_ENABLE_VPC
  //g->notifyReset();
  g->doReset();
#ifdef SYSTEMOC_ENABLE_SGX
  if (getSimCTX()->isSMXDumpingPreSimEnabled()) {
    SysteMoC::Detail::dumpSMX(getSimCTX()->getSMXPreSimFile(), getSimCTX(), *g);
    if (!getSimCTX()->isSMXDumpingPreSimKeepGoing())
      sc_core::sc_stop();
  }
#endif // SYSTEMOC_ENABLE_SGX
#ifdef SYSTEMOC_NEED_IDS
  getSimCTX()->generateIdsAfterFinalise();
#endif // SYSTEMOC_NEED_IDS
}

void smoc_scheduler_top::schedule() {
  if (getSimCTX()->isVpcSchedulingEnabled()) {
    return;
//      std::cerr << "SMoC: " << getSimCTX()->isVpcSchedulingEnabled() << std::endl;
      smoc_node_list nodes;
      g->getNodesRecursive(nodes);

      boost::function<void (SystemC_VPC::ScheduledTask* actor)> callExecute
          = &SysteMoC::Scheduling::execute;
      boost::function<bool (SystemC_VPC::ScheduledTask* actor)> callCanExecute
          = &SysteMoC::Scheduling::canExecute;

      CoSupport::SystemC::EventOrList<smoc_root_node> actors;
      for (smoc_node_list::iterator iter = nodes.begin();
          iter != nodes.end(); ++iter){
          smoc_root_node *actor = *iter;
          actors |= *actor;
      }

      while(true) {
        smoc_wait(actors);
        for (smoc_node_list::iterator iter = nodes.begin();
            iter != nodes.end(); ++iter){
            smoc_root_node *node = *iter;
            SystemC_VPC::ScheduledTask * actor =
                dynamic_cast<SystemC_VPC::ScheduledTask *>(node);
            assert(actor != NULL);
            std::cerr << node->name() << " : " << callCanExecute(actor) << std::endl;
            if (callCanExecute(actor)) {
                callExecute(actor);
            }
        }
      }
      return;
  }

  // plain old SysteMoC scheduler
  while(true) {
    smoc_wait(*g);
    while(*g) {
#ifdef SYSTEMOC_DEBUG
      outDbg << "<node name=\"" << g->name() << "\">" << std::endl
             << Indent::Up;
#endif // SYSTEMOC_DEBUG
      g->schedule();
#ifdef SYSTEMOC_DEBUG
      outDbg << Indent::Down << "</node>" << std::endl;
#endif // SYSTEMOC_DEBUG
    }
  }
}

