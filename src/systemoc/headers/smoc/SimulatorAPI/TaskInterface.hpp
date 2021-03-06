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

#ifndef _INCLUDED_SMOC_SIMULATORAPI_TASKINTERFACE_HPP
#define _INCLUDED_SMOC_SIMULATORAPI_TASKINTERFACE_HPP

#include "FiringRuleInterface.hpp"

#include <systemc>

#include <list>

namespace smoc {

  class smoc_periodic_actor;

} // namespace smoc

namespace smoc { namespace Detail {

  class NodeBase;
  class PortInBase;
  class PortOutBase;

} } // namespace smoc::Detail

namespace smoc { namespace Detail { namespace FSM {

  class FiringFSM;
  class RuntimeTransition;

} } } // namespace smoc::Detail::FSM

namespace smoc { namespace SimulatorAPI {

  class SchedulerInterface;
  class TaskHandle;

  class TaskInterface {
  private:
    SchedulerInterface *scheduler;
    // Opaque data pointer for the scheduler.
    void               *schedulerInfo;
  public:
    // These methods are called by the scheduler
    void                setScheduler(SchedulerInterface *scheduler)
      { this->scheduler = scheduler; }
    SchedulerInterface *getScheduler() const
      { return this->scheduler; }

    void                setSchedulerInfo(void *schedulerInfo)
      { this->schedulerInfo = schedulerInfo; }
    void               *getSchedulerInfo() const
      { return this->schedulerInfo; }

    // These methods must be implemented by the task and are called by the scheduler

    virtual std::list<FiringRuleInterface *> const &getFiringRules() = 0;

    // This will return the SystemC name of the actor.
    virtual const char *name() const = 0;

    // This will execute the actor. The actor must be fireable if this method is called.
    // This will be implemented by the SysteMoC actor and called by the scheduler.
    virtual void schedule() = 0;

    // This will test if the actor is fireable.
    // This will be implemented by the SysteMoC actor and called by the scheduler.
    virtual bool canFire()  = 0;

    // This method will be implemented by the SysteMoC actor and is used to switch between
    // usage of the notifyActivation callback and between polling mode performed via canFire.
    //
    // setUseActivationCallback(false) will disable the notifyActivation callback.
    // setUseActivationCallback(true)  will enable the notifyActivation callback.
    virtual void setUseActivationCallback(bool flags) = 0;

    // This method will be implemented by SysteMoC and is used to query the status
    // of the notifyActivation callback. If this method returns true, then SysteMoC will call
    // notifyActivation to notify the SchedulerInterface that an actor is fireable.
    // Otherwise, this has to be check via calls to canFire.
    virtual bool getUseActivationCallback() const = 0;

    // This must return a time until the actor is suspended
    // due to some timing behavior. If no timing behavior is
    // given, then this should return sc_core::sc_time_stamp().
    // This will be implemented by the SysteMoC actor and called by the scheduler.
    virtual sc_core::sc_time const &getNextReleaseTime() const = 0;

    // This method will be implemented by SysteMoC and can be used
    // to enable (true) or disable (false) the scheduling of the
    // SysteMoC actor.
    virtual void setActive(bool) = 0;

    // This method will be implemented by SysteMoC and is the
    // corresponding getter for the setActive method.
    virtual bool getActive() const = 0;

#ifdef SYSTEMOC_SIMULATOR_COUPLING_COMPILATION
    operator TaskHandle       &();
    operator TaskHandle const &() const;
#endif //defined(SYSTEMOC_SIMULATOR_COUPLING_COMPILATION)
  protected:
    TaskInterface()
      : scheduler(nullptr)
      , schedulerInfo(nullptr) {}

    virtual ~TaskInterface() {}
  };

  class TaskHandle
#ifdef SYSTEMOC_SIMULATOR_COUPLING_COMPILATION
    : public TaskInterface
#else //!defined(SYSTEMOC_SIMULATOR_COUPLING_COMPILATION)
    : private TaskInterface
#endif //!defined(SYSTEMOC_SIMULATOR_COUPLING_COMPILATION)
  {
    friend class smoc::smoc_periodic_actor;
    friend class smoc::Detail::NodeBase;
    friend class smoc::Detail::PortInBase;
    friend class smoc::Detail::PortOutBase;
    friend class smoc::Detail::FSM::FiringFSM; // for getScheduler()
    friend class smoc::Detail::FSM::RuntimeTransition; // for getScheduler()
  protected:
    virtual ~TaskHandle() {}
  };

#ifdef SYSTEMOC_SIMULATOR_COUPLING_COMPILATION
  inline
  TaskInterface::operator TaskHandle       &()
    { return *static_cast<TaskHandle       *>(this); }

  inline
  TaskInterface::operator TaskHandle const &() const
    { return *static_cast<TaskHandle const *>(this); }
#endif //defined(SYSTEMOC_SIMULATOR_COUPLING_COMPILATION)

} } // namespace smoc::SimulatorAPI

#endif /* _INCLUDED_SMOC_SIMULATORAPI_TASKINTERFACE_HPP */
