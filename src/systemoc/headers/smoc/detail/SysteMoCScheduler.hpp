/*
 * SysteMoCScheduler.hpp
 *
 *  Created on: 09.05.2017
 *      Author: muellersi
 */

#ifndef _INCLUDED_SMOC_DETAIL_SYSTEMOCSCHEDULER_HPP
#define _INCLUDED_SMOC_DETAIL_SYSTEMOCSCHEDULER_HPP

#include <smoc/EvalAPI/SchedulingInterface.hpp>

namespace smoc {

  class smoc_graph;

} // namespace smoc

namespace smoc { namespace Detail {

  class SysteMoCScheduler
    : protected EvalAPI::SchedulingInterface
    , public sc_core::sc_module
  {
    SC_HAS_PROCESS(SysteMoCScheduler);
  protected:
    SysteMoCScheduler(sc_core::sc_module_name);

    // This will be called by SysteMoC if useActivationCallback()
    // return true.
    void setActivation(bool activation);

  private:
    /// This event will be notified by setActivation if
    /// no VPC or Maestro scheduling is activated to
    /// enable SysteMoC self scheduling of the actor.
    sc_core::sc_event scheduleRequest;

    void scheduleRequestMethod();
  };

} } // namespace smoc::Detail

#endif // _INCLUDED_SMOC_DETAIL_SYSTEMOCSCHEDULER_HPP
