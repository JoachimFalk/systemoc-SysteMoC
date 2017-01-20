//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:
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

#include <smoc/smoc_scheduler_top.hpp>

#include <CoSupport/compatibility-glue/nullptr.h>

#include <systemoc/smoc_config.h>

#ifdef SYSTEMOC_ENABLE_VPC
# include <systemcvpc/Director.hpp>
#endif //SYSTEMOC_ENABLE_VPC

#include <smoc/detail/DebugOStream.hpp>
#include <smoc/detail/GraphBase.hpp>

#include "SMXDumper.hpp"

namespace smoc {

#ifdef SYSTEMOC_ENABLE_VPC
namespace Detail {

  static
  bool systemcVpcCanExecute(SystemC_VPC::ScheduledTask *actor) {
    assert(dynamic_cast<smoc_actor*>(actor) != nullptr);
    return static_cast<smoc_actor*>(actor)->canFire();
  }

  static
  void systemcVpcExecute(SystemC_VPC::ScheduledTask *actor) {
    //std::cerr << "smoc::Scheduling::execute" << std::endl;
    assert(dynamic_cast<smoc_actor*>(actor) != nullptr);
    static_cast<smoc_actor*>(actor)->schedule();
  }

} // Detail
#endif //SYSTEMOC_ENABLE_VPC

smoc_scheduler_top::smoc_scheduler_top(Detail::GraphBase *g) :
  // Prefix all SysteMoC internal modules with __smoc_ to enable filtering out the module on smx dump!
  sc_core::sc_module(sc_core::sc_module_name("__smoc_smoc_scheduler_top")),
  g(g),
  validVpcConfiguration(false),
  simulation_running(false)
{
  this->g->setScheduler(this);
  SC_THREAD(schedule);
}

smoc_scheduler_top::smoc_scheduler_top(Detail::GraphBase &g) :
  // Prefix all SysteMoC internal modules with __smoc_ to enable filtering out the module on smx dump!
  sc_core::sc_module(sc_core::sc_module_name("__smoc_smoc_scheduler_top")),
  g(&g),
  simulation_running(false)
{
  this->g->setScheduler(this);
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
    Detail::dumpSMX(getSimCTX()->getSMXPostSimFile(), getSimCTX(), *g);
  }
#endif // SYSTEMOC_ENABLE_SGX
  getSimCTX()->endOfSystemcSimulation();
}

void smoc_scheduler_top::_before_end_of_elaboration() {
#ifdef SYSTEMOC_DEBUG
  if (Detail::outDbg.isVisible(Detail::Debug::High)) {
    Detail::outDbg << "<smoc_scheduler_top::_before_end_of_elaboration name=\"" << this->name() << "\">"
         << std::endl << Detail::Indent::Up;
  }
#endif //defined(SYSTEMOC_DEBUG)
  try {
#ifdef SYSTEMOC_ENABLE_VPC
    SystemC_VPC::Director::getInstance().beforeVpcFinalize();
    
    boost::function<void (SystemC_VPC::ScheduledTask *actor)> callExecute
      = &Detail::systemcVpcExecute;
    boost::function<bool (SystemC_VPC::ScheduledTask *actor)> callCanExecute
      = &Detail::systemcVpcCanExecute;
    SystemC_VPC::Director::getInstance().
      registerSysteMoCCallBacks(callExecute, callCanExecute);
#endif //SYSTEMOC_ENABLE_VPC
#ifdef SYSTEMOC_ENABLE_MAESTRO
    MM::MMAPI* api = MM::MMAPI::getInstance();
    api->beforeEndOfElaboration();
#endif //SYSTEMOC_ENABLE_MAESTRO
  } catch (std::exception &e) {
    std::cerr << "Got exception at smoc_scheduler_top::_before_end_of_elaboration():\n\t"
              << e.what();
    exit(-1);
  }
#ifdef SYSTEMOC_DEBUG
  if (Detail::outDbg.isVisible(Detail::Debug::High)) {
    Detail::outDbg << Detail::Indent::Down << "</smoc_scheduler_top::_before_end_of_elaboration>"
         << std::endl;
  }
#endif //defined(SYSTEMOC_DEBUG)
}

void smoc_scheduler_top::_end_of_elaboration() {
#ifdef SYSTEMOC_DEBUG
  if (Detail::outDbg.isVisible(Detail::Debug::High)) {
    Detail::outDbg << "<smoc_scheduler_top::_end_of_elaboration name=\"" << this->name() << "\">"
         << std::endl << Detail::Indent::Up;
  }
#endif //defined(SYSTEMOC_DEBUG)
  try {
#ifdef SYSTEMOC_ENABLE_VPC
    SystemC_VPC::Director::getInstance().endOfVpcFinalize();
    validVpcConfiguration = SystemC_VPC::Director::getInstance().hasValidConfig();
#endif //SYSTEMOC_ENABLE_VPC
#ifdef SYSTEMOC_ENABLE_MAESTRO
    MM::MMAPI* api = MM::MMAPI::getInstance();
    api->endOfElaboration();
#endif //SYSTEMOC_ENABLE_MAESTRO
//  g->doReset();
#ifdef SYSTEMOC_ENABLE_SGX
    if (getSimCTX()->isSMXDumpingPreSimEnabled()) {
      Detail::dumpSMX(getSimCTX()->getSMXPreSimFile(), getSimCTX(), *g);
      if (!getSimCTX()->isSMXDumpingPreSimKeepGoing())
        sc_core::sc_stop();
    }
#endif // SYSTEMOC_ENABLE_SGX
  } catch (std::exception &e) {
    std::cerr << "Got exception at smoc_scheduler_top::_end_of_elaboration():\n\t"
              << e.what();
    exit(-1);
  }
#ifdef SYSTEMOC_DEBUG
  if (Detail::outDbg.isVisible(Detail::Debug::High)) {
    Detail::outDbg << Detail::Indent::Down << "</smoc_scheduler_top:::_end_of_elaboration>"
         << std::endl;
  }
#endif //defined(SYSTEMOC_DEBUG)
}

void smoc_scheduler_top::schedule() {
#ifdef SYSTEMOC_ENABLE_MAESTRO
  return;
#endif //SYSTEMOC_ENABLE_MAESTRO
#ifdef SYSTEMOC_ENABLE_VPC
  // enable VPC scheduling if the VPC configuration is valid
  // or if forced by command line option
  if (validVpcConfiguration || getSimCTX()->isVpcSchedulingEnabled())
    return;
#endif //SYSTEMOC_ENABLE_VPC
  // plain old smoc scheduler
  while (true) {
    smoc_wait(*g);
    while (*g) {
#ifdef SYSTEMOC_DEBUG
      if (Detail::outDbg.isVisible(Detail::Debug::Medium)) {
        Detail::outDbg << "<node name=\"" << g->name() << "\">" << std::endl
             << Detail::Indent::Up;
      }
#endif // SYSTEMOC_DEBUG
      g->schedule();
#ifdef SYSTEMOC_DEBUG
      if (Detail::outDbg.isVisible(Detail::Debug::Medium)) {
        Detail::outDbg << Detail::Indent::Down << "</node>" << std::endl;
      }
#endif // SYSTEMOC_DEBUG
    }
  }
}

} // namespace smoc
