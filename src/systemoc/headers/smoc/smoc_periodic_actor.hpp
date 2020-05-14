// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2012 FAU -- Sebastian Graf <sebastian.graf@fau.de>
 *   2013 FAU -- Sebastian Graf <sebastian.graf@fau.de>
 *   2013 Thomas Russ <tr.thomas.russ@googlemail.com>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Matthias Schid <matthias.schid@fau.de>
 *   2017 FAU -- Simone Müller <simone.mueller@fau.de>
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
                smoc_state &start_state,
                sc_core::sc_time per,
                sc_core::sc_time off,
                float jitter=0.0);

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

  bool canFire();
  void schedule();
  void updateReleaseTime();

  sc_core::sc_time       calculateMobility() const;
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

#endif /* _INCLUDED_SMOC_SMOC_PERIODIC_ACTOR_HPP */
