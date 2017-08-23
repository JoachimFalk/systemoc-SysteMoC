/*
 * SysteMoCScheduler.cpp
 *
 *  Created on: 09.05.2017
 *      Author: muellersi
 */

#include <smoc/detail/SysteMoCScheduler.hpp>

namespace smoc { namespace Detail {

SysteMoCScheduler::SysteMoCScheduler(sc_core::sc_module_name name)
  : sc_core::sc_module(name)
{
  SC_METHOD(scheduleRequestMethod);
  sensitive << scheduleRequest;
  dont_initialize();
}

void SysteMoCScheduler::scheduleRequestMethod() {
  while (getActive() && canFire())
    schedule();
}

void SysteMoCScheduler::setActivation(bool activation) {
  if (activation)
    scheduleRequest.notify(getNextReleaseTime()-sc_core::sc_time_stamp());
  else
    scheduleRequest.cancel();
}

} } // namespace smoc::Detail
