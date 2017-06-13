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

#ifndef _INCLUDED_SMOC_SMOC_PERIODIC_ACTOR_HPP
#define _INCLUDED_SMOC_SMOC_PERIODIC_ACTOR_HPP

#include "smoc_actor.hpp"

namespace smoc {

/*class smoc_periodic_actor 
 * used to generate an event periodically.
 * the period, its offset 
 * and a pointer to the managing EventQueue is passed to the Constructor*/
class smoc_periodic_actor: public smoc_actor {
public:
  //constructor sets the period, offset and EventQueue
  smoc_periodic_actor(sc_core::sc_module_name name,
                smoc_firing_state & start_state,
                sc_core::sc_time per,
                sc_core::sc_time off,
                float jitter=0.0);

  sc_core::sc_time calculateMobility() const;

  void             updateReleaseTime();

  sc_core::sc_time getPeriod()
    { return period; }

  sc_core::sc_time getOffset()
    { return offset; }

protected:

  void forceReexecution()
    { reexecute = true; }

  void stopPeriodicActorExecution()
    { periodicActorActive = false; }

  void restartPeriodicActorExecution()
    { periodicActorActive = true; }

private:

  void schedule();

  bool canFire();

  sc_core::sc_time const &getNextReleaseTime() const;

  int period_counter;
  sc_core::sc_time period;
  sc_core::sc_time offset;
  sc_core::sc_time nextReleaseTime;
  float jitter;
  bool reexecute;
  bool periodicActorActive;

};

} // namespace smoc

#endif // _INCLUDED_SMOC_SMOC_PERIODIC_ACTOR_HPP
