/*
 * SchedulingInterface.hpp
 *
 *  Created on: 09.05.2017
 *      Author: muellersi
 */

#ifndef _INCLUDED_SMOC_EVALAPI_SCHEDULINGINTERFACE_HPP
#define _INCLUDED_SMOC_EVALAPI_SCHEDULINGINTERFACE_HPP

#include <systemc>

namespace smoc { namespace EvalAPI {

  class SchedulingInterface {
  public:
    // This will execute the actor. The actor must be fireable if this method is called.
    // This will be implemented by the SysteMoC actor and called by the scheduler.
    virtual void schedule() = 0;
    // This will test if the actor is fireable.
    // This will be implemented by the SysteMoC actor and called by the scheduler.
    virtual bool canFire()  = 0;

    // If this method returns true, then SysteMoC will call
    // setActivation to notify the SchedulingInterface
    // that an actor is fireable. Otherwise, this has
    // to be check via calls to canFire.
    virtual bool useActivationCallback() const = 0;

    // This will be called by SysteMoC if useActivationCallback()
    // return true.
    virtual void setActivation(bool activation) = 0;

    // This must return a time until the actor is suspended
    // due to some timing behavior. If no timing behavior is
    // given, then this should return sc_core::sc_time_stamp().
    // This will be implemented by the SysteMoC actor and called by the scheduler.
    virtual sc_core::sc_time const &getNextReleaseTime() const = 0;

    // setActive(false) will disable the scheduling of this actor
    // setActive(true) will enable the scheduling of this actor
    virtual void setActive(bool) = 0;
    virtual bool getActive() const = 0;

    virtual ~SchedulingInterface() {};
  };

} } // namespace smoc::EvalAPI

#endif // _INCLUDED_SMOC_EVALAPI_SCHEDULINGINTERFACE_HPP
