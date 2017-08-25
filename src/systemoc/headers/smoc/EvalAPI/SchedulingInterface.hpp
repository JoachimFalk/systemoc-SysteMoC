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
    // FIXME: Remove this interface after SystemC-VPC has been modified to
    // always use the schedule call above.
    //
    // This will execute the actor. The actor must be fireable if this method is called.
    // This will be implemented by the SysteMoC actor and called by the scheduler.
    // In comparison to the schedule method this method will insert the commState
    // into every transition. The commState is left if the DII event is notified
    // by SystemC-VPC.
    virtual void scheduleLegacyWithCommState() = 0;

    // This will test if the actor is fireable.
    // This will be implemented by the SysteMoC actor and called by the scheduler.
    virtual bool canFire()  = 0;

    // This method will be implemented by the SysteMoC actor and is used to switch between
    // usage of the setActivation callback and between polling mode performed via canFire.
    //
    // setUseActivationCallback(false) will disable the setActivation callback.
    // setUseActivationCallback(true)  will enable the setActivation callback.
    virtual void setUseActivationCallback(bool flags) = 0;

    // This method will be implemented by SysteMoC and is used to query the status
    // of the setActivation callback. If this method returns true, then SysteMoC will call
    // setActivation to notify the SchedulingInterface that an actor is fireable.
    // Otherwise, this has to be check via calls to canFire.
    virtual bool getUseActivationCallback() const = 0;

    // This must be implemented by the scheduler and will be called by SysteMoC
    // if getUseActivationCallback() returns true.
    virtual void setActivation(bool activation) = 0;

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

    virtual ~SchedulingInterface() {};
  };

} } // namespace smoc::EvalAPI

#endif // _INCLUDED_SMOC_EVALAPI_SCHEDULINGINTERFACE_HPP
