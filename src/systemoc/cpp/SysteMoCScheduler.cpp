/*
 * SysteMoCScheduler.cpp
 *
 *  Created on: 09.05.2017
 *      Author: muellersi
 */

#include <smoc/detail/SysteMoCScheduler.hpp>

namespace smoc { namespace Detail {

SysteMoCScheduler::SysteMoCScheduler(sc_core::sc_module_name name)
  : sc_core::sc_module(name), active(true)
{
  SC_METHOD(scheduleRequestMethod);
  sensitive << scheduleRequest;
  dont_initialize();
}

void SysteMoCScheduler::scheduleRequestMethod() {
  while (canFire())
    schedule();
}

void SysteMoCScheduler::setActivation(bool activation) {
  if (activation)
    scheduleRequest.notify(getNextReleaseTime()-sc_core::sc_time_stamp());
  else
    scheduleRequest.cancel();
}

void SysteMoCScheduler::setActive(bool active)
  { assert(!"Implement this"); this->active = active; }

} } // namespace smoc::Detail
