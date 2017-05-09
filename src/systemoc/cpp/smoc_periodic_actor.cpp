/*
 * Copyright (c) 2004-2017 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 * 
 * --- This software and any associated documentation is provided "as is" 
 * 
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 */

#include <smoc/smoc_periodic_actor.hpp>

namespace smoc {

//constructor sets the period, offset and EventQueue
smoc_periodic_actor::smoc_periodic_actor(
    sc_core::sc_module_name name, smoc_firing_state &start_state,
    sc_core::sc_time per, sc_core::sc_time off, float jitter)
  : smoc_actor(name, start_state)
  , period_counter(0)
  , period(per), offset(off), nextReleaseTime_(off), jitter(jitter), reexecute(false)
  , periodicActorActive(true)
{
  // TODO: negative mobility values may result in negative release times
  //nextReleaseTime_ += calculateMobility();
}

sc_core::sc_time smoc_periodic_actor::calculateMobility() const {
  sc_core::sc_time mobility = sc_core::SC_ZERO_TIME;
  if (jitter != 0.0) {
    mobility = jitter * ((rand() / (float(RAND_MAX) + 1) * 2) - 1) * period;
  }
  return mobility;
}

sc_core::sc_time smoc_periodic_actor::updateReleaseTime() {
  while (nextReleaseTime_ <= sc_core::sc_time_stamp()) {
    period_counter++; // increment first, initial execution is scheduled @ offset
    nextReleaseTime_ = period_counter * period + offset + calculateMobility();
  }
  return nextReleaseTime_;
}

// override getNextReleaseTime from ScheduledTask
sc_core::sc_time smoc_periodic_actor::getNextReleaseTime() {
  if (reexecute) {
    reexecute = false;
    return sc_core::sc_time_stamp();
  } else {
    if (periodicActorActive) {
      return updateReleaseTime();
    } else {
      periodicActorActive = true;
      return sc_core::sc_time_stamp();
   }
  }
}

void smoc_periodic_actor::setActivation(bool activation, sc_core::sc_time const &delta) {
  if (activation
#ifdef SYSTEMOC_ENABLE_VPC
      && !this->inCommState()
#endif //SYSTEMOC_ENABLE_VPC
     )
    smoc_actor::setActivation(activation,
        getNextReleaseTime() - sc_core::sc_time_stamp());
  else
    smoc_actor::setActivation(activation, delta);
}

} // namesdpace smoc
