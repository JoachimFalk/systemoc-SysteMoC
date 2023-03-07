// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2012 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2012 FAU -- Liyuan Zhang <liyuan.zhang@cs.fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#include "RuntimeState.hpp"
#include "../SimulationContext.hpp"

namespace smoc { namespace Detail { namespace FSM {

  RuntimeState::RuntimeState(std::string const &name)
    : stateName(name)
  {
    assert(!name.empty());
#ifdef SYSTEMOC_NEED_IDS
    // Allocate Id for myself.
    getSimCTX()->getIdPool().addIdedObj(this);
#endif // SYSTEMOC_NEED_IDS
  }

  RuntimeState::~RuntimeState() {
  }

  const RuntimeTransitionList &RuntimeState::getTransitions() const
    { return tl; }

  RuntimeTransitionList& RuntimeState::getTransitions()
    { return tl; }

  void RuntimeState::addTransition(RuntimeTransition const &t) {
    tl.push_back(t);
  }

  void RuntimeState::end_of_elaboration() {
    for (RuntimeTransition &t : tl)
      am.insert(t.getIOPatternWaiter());
  }

} } } // namespace smoc::Detail::FSM
