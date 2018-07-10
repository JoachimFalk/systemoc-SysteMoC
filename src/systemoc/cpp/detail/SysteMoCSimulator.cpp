// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * SysteMoCScheduler.cpp
 *
 *  Created on: 09.05.2017
 *      Author: muellersi
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

void SysteMoCSimulator::registerTask(TaskInterface *task)
{
  // Prefix all SysteMoC internal modules with __smoc_ to enable filtering out the module on smx dump!
  task->setScheduler(new SysteMoCScheduler("__smoc_scheduler", task));
}

void SysteMoCSimulator::registerPort(
    TaskInterface    *task,
    PortInInterface  *port)
{
}

void SysteMoCSimulator::registerPort(
    TaskInterface    *task,
    PortOutInterface *port)
{
}

void SysteMoCSimulator::simulationEnded() {
}

SysteMoCSimulator::~SysteMoCSimulator() {

}

} } // namespace smoc::Detail
