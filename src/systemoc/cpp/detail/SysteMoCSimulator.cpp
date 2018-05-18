// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * SysteMoCScheduler.cpp
 *
 *  Created on: 09.05.2017
 *      Author: muellersi
 */

#include <smoc/SimulatorAPI/SchedulerInterface.hpp>
#include <smoc/SimulatorAPI/TaskInterface.hpp>
#include <smoc/SimulatorAPI/SimulatorInterface.hpp>

namespace smoc { namespace Detail {

class SysteMoCScheduler
  : public sc_core::sc_module
  , public SimulatorAPI::SchedulerInterface
{
  SC_HAS_PROCESS(SysteMoCScheduler);
public:
  SysteMoCScheduler(sc_core::sc_module_name, SimulatorAPI::TaskInterface *task);

  // This will be called by SysteMoC if useActivationCallback()
  // return true.
  void notifyActivation(SimulatorAPI::TaskInterface *task, bool activation);

private:
  SimulatorAPI::TaskInterface *task;

  /// This event will be notified by notifyActivation if
  /// no VPC or Maestro scheduling is activated to
  /// enable SysteMoC self scheduling of the actor.
  sc_core::sc_event scheduleRequest;

  void scheduleRequestMethod();
};


SysteMoCScheduler::SysteMoCScheduler(sc_core::sc_module_name name, SimulatorAPI::TaskInterface *task)
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

void SysteMoCScheduler::notifyActivation(SimulatorAPI::TaskInterface *task, bool activation) {
  assert(task == this->task);
  if (activation)
    scheduleRequest.notify(task->getNextReleaseTime()-sc_core::sc_time_stamp());
  else
    scheduleRequest.cancel();
}

class SysteMoCSimulator
  : public SimulatorAPI::SimulatorInterface
{
public:
  SysteMoCSimulator();

  void populateOptionsDescription(
      int &argc, char ** &argv,
      boost::program_options::options_description &pub,
      boost::program_options::options_description &priv);

  EnablementStatus evaluateOptionsMap(
      boost::program_options::variables_map &vm);

  void registerTask(SimulatorAPI::TaskInterface *task);
};

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
    SimulatorAPI::TaskInterface *task)
{
  // Prefix all SysteMoC internal modules with __smoc_ to enable filtering out the module on smx dump!
  task->setScheduler(new SysteMoCScheduler("__smoc_scheduler", task));
}

SysteMoCSimulator systeMoCSimulator;

} } // namespace smoc::Detail
