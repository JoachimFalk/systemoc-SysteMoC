// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2019 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#include "SysteMoCSimulator.hpp"

#include <smoc/SimulatorAPI/SchedulerInterface.hpp>
#include <smoc/SimulatorAPI/TaskInterface.hpp>
#include <smoc/SimulatorAPI/FiringRuleInterface.hpp>
#include <smoc/SimulatorAPI/SimulatorInterface.hpp>

#include <smoc/detail/PortBase.hpp>

#include <systemoc/smoc_config.h>

namespace smoc { namespace Detail {

class SysteMoCScheduler
  : public sc_core::sc_module
  , public SimulatorAPI::SchedulerInterface
{
  SC_HAS_PROCESS(SysteMoCScheduler);

  typedef SimulatorAPI::TaskInterface           TaskInterface;
  typedef SimulatorAPI::FiringRuleInterface     FiringRuleInterface;
  typedef FiringRuleInterface::PortInInfo       PortInInfo;
  typedef FiringRuleInterface::PortOutInfo      PortOutInfo;
public:
  SysteMoCScheduler(sc_core::sc_module_name, TaskInterface *task);

  // This will be called by SysteMoC if useActivationCallback()
  // return true.
  void notifyActivation(TaskInterface *task, bool activation);

  void registerFiringRule(TaskInterface *task, FiringRuleInterface *fr);

  void checkFiringRule(TaskInterface *task, FiringRuleInterface *fr);

  void executeFiringRule(TaskInterface *task, FiringRuleInterface *fr);

private:
  TaskInterface *task;

  /// This event will be notified by notifyActivation if
  /// no VPC or Maestro scheduling is activated to
  /// enable SysteMoC self scheduling of the actor.
  sc_core::sc_event scheduleRequest;

  void scheduleRequestMethod();
};


SysteMoCScheduler::SysteMoCScheduler(sc_core::sc_module_name name, TaskInterface *task)
  : sc_core::sc_module(name), task(task)
{
  SC_METHOD(scheduleRequestMethod);
  sensitive << scheduleRequest;
  dont_initialize();
}

void SysteMoCScheduler::scheduleRequestMethod() {
  while (task->getActive() && task->canFire())
    task->schedule();
}

void SysteMoCScheduler::notifyActivation(TaskInterface *task, bool activation) {
  assert(task == this->task);
  if (activation)
    scheduleRequest.notify(task->getNextReleaseTime()-sc_core::sc_time_stamp());
  else
    scheduleRequest.cancel();
}

void SysteMoCScheduler::registerFiringRule(TaskInterface *task, FiringRuleInterface *fr) {

}

void SysteMoCScheduler::checkFiringRule(TaskInterface *task, FiringRuleInterface *fr) {

}

void SysteMoCScheduler::executeFiringRule(TaskInterface *task, FiringRuleInterface *fr) {
  for (PortInInfo const &portInfo : fr->getPortInInfos()) {
    portInfo.port.commExec(portInfo.consumed);
  }
  for (PortOutInfo const &portInfo : fr->getPortOutInfos()) {
    portInfo.port.commExec(portInfo.produced);
  }
}

SysteMoCSimulator::SysteMoCSimulator() {
}

void SysteMoCSimulator::populateOptionsDescription(
    int &argc, char ** &argv,
    boost::program_options::options_description &pub,
    boost::program_options::options_description &priv)
{
  // No SysteMoC simulator specific options
}

SysteMoCSimulator::EnablementStatus SysteMoCSimulator::evaluateOptionsMap(
    boost::program_options::variables_map &vm)
{
  return SysteMoCSimulator::MAYBE_ACTIVE;
}

void SysteMoCSimulator::registerTask(
    TaskInterface                          *task,
    std::list<FiringRuleInterface *> const &firingRules)
{
  // Prefix all SysteMoC internal modules with __smoc_ to enable filtering out the module on smx dump!
  task->setScheduler(new SysteMoCScheduler("__smoc_scheduler", task));
}

void SysteMoCSimulator::registerPort(PortInInterface  *port)
{
}

void SysteMoCSimulator::registerPort(PortOutInterface *port)
{
}

SysteMoCSimulator::~SysteMoCSimulator() {
}

} } // namespace smoc::Detail
