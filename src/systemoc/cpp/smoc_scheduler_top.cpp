// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2012 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2012 FAU -- Rafael Rosales <rafael.rosales@fau.de>
 *   2013 FAU -- Liyuan Zhang <liyuan.zhang@cs.fau.de>
 *   2015 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Simone Müller <simone.mueller@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2019 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
 * 
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option) any
 * later version.
 * 
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#include <smoc/smoc_scheduler_top.hpp>

#include <systemoc/smoc_config.h>

//#ifdef SYSTEMOC_ENABLE_VPC
//# include <vpc.hpp>
//#endif //SYSTEMOC_ENABLE_VPC

#include <smoc/detail/DebugOStream.hpp>
#include <smoc/detail/GraphBase.hpp>

#include "detail/SMXDumper.hpp"
#include "detail/SNGDumper.hpp"
#include "detail/SimulationContext.hpp"

namespace smoc {

smoc_scheduler_top::smoc_scheduler_top(Detail::GraphBase *g) :
  // Prefix all SysteMoC internal modules with __smoc_ to enable filtering out the module on smx dump!
  sc_core::sc_module(sc_core::sc_module_name("__smoc_smoc_scheduler_top")),
  g(g),
  simulation_running(false)
{
  this->g->setScheduler(this);
//SC_THREAD(schedule);
}

smoc_scheduler_top::smoc_scheduler_top(Detail::GraphBase &g) :
  // Prefix all SysteMoC internal modules with __smoc_ to enable filtering out the module on smx dump!
  sc_core::sc_module(sc_core::sc_module_name("__smoc_smoc_scheduler_top")),
  g(&g),
  simulation_running(false)
{
  this->g->setScheduler(this);
//SC_THREAD(schedule);
}

smoc_scheduler_top::~smoc_scheduler_top() {
  if(simulation_running)
    sc_core::sc_stop();
}

void smoc_scheduler_top::start_of_simulation() {
#ifdef SYSTEMOC_ENABLE_SGX
  if (getSimCTX()->isSMXDumpingPreSimEnabled()) {
    // Note that doReset() of each actor, graph, and channel must have been done before starting the dumping.
    Detail::dumpSMX(getSimCTX()->getSMXPreSimFile(), getSimCTX(), *g);
    if (!getSimCTX()->isSMXDumpingPreSimKeepGoing())
      sc_core::sc_stop();
  }
#endif // SYSTEMOC_ENABLE_SGX
  if (getSimCTX()->isSNGDumpingEnabled()) {
    // Note that doReset() of each actor, graph, and channel must have been done before starting the dumping.
    Detail::dumpSNG(getSimCTX()->getSNGDumpFile(), getSimCTX(), *g);
    sc_core::sc_stop();
  }
  simulation_running = true;
}

void smoc_scheduler_top::end_of_simulation() {
  simulation_running = false;
#ifdef SYSTEMOC_ENABLE_SGX
  if (getSimCTX()->isSMXDumpingPostSimEnabled()) {
    Detail::dumpSMX(getSimCTX()->getSMXPostSimFile(), getSimCTX(), *g);
  }
#endif // SYSTEMOC_ENABLE_SGX
}

void smoc_scheduler_top::_before_end_of_elaboration() {
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (Detail::outDbg.isVisible(Detail::Debug::High)) {
    Detail::outDbg << "<smoc_scheduler_top::_before_end_of_elaboration name=\"" << this->name() << "\">"
         << std::endl << Detail::Indent::Up;
  }
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
  try {
#ifdef SYSTEMOC_ENABLE_SGX
    Detail::importSMX(getSimCTX());
#endif // SYSTEMOC_ENABLE_SGX
//#ifdef SYSTEMOC_ENABLE_VPC
//    SystemC_VPC::Director::getInstance().beforeVpcFinalize();
//#endif //SYSTEMOC_ENABLE_VPC
#ifdef SYSTEMOC_ENABLE_MAESTRO
    MM::MMAPI* api = MM::MMAPI::getInstance();
    api->beforeEndOfElaboration();
#endif //SYSTEMOC_ENABLE_MAESTRO
  } catch (std::exception &e) {
    std::cerr << "Got exception at smoc_scheduler_top::_before_end_of_elaboration():\n\t"
              << e.what();
    exit(-1);
  }
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (Detail::outDbg.isVisible(Detail::Debug::High)) {
    Detail::outDbg << Detail::Indent::Down << "</smoc_scheduler_top::_before_end_of_elaboration>"
         << std::endl;
  }
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
}

void smoc_scheduler_top::_end_of_elaboration() {
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (Detail::outDbg.isVisible(Detail::Debug::High)) {
    Detail::outDbg << "<smoc_scheduler_top::_end_of_elaboration name=\"" << this->name() << "\">"
         << std::endl << Detail::Indent::Up;
  }
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
  try {
//#ifdef SYSTEMOC_ENABLE_VPC
//    SystemC_VPC::Director::getInstance().endOfVpcFinalize();
//#endif //SYSTEMOC_ENABLE_VPC
#ifdef SYSTEMOC_ENABLE_MAESTRO
    MM::MMAPI* api = MM::MMAPI::getInstance();
    api->endOfElaboration();
#endif //SYSTEMOC_ENABLE_MAESTRO
  } catch (std::exception &e) {
    std::cerr << "Got exception at smoc_scheduler_top::_end_of_elaboration():\n\t"
              << e.what();
    exit(-1);
  }
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (Detail::outDbg.isVisible(Detail::Debug::High)) {
    Detail::outDbg << Detail::Indent::Down << "</smoc_scheduler_top:::_end_of_elaboration>"
         << std::endl;
  }
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
}

} // namespace smoc
