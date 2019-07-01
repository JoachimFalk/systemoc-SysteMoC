// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2019 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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

  RuntimeState::RuntimeState(std::string const &name
#if defined(SYSTEMOC_ENABLE_MAESTRO) && defined(MAESTRO_ENABLE_BRUCKNER)
      , Bruckner::Model::Hierarchical* sParent = nullptr
#endif//defined(SYSTEMOC_ENABLE_MAESTRO) && defined(MAESTRO_ENABLE_BRUCKNER)
    ) :
#if defined(SYSTEMOC_ENABLE_MAESTRO) && defined(MAESTRO_ENABLE_BRUCKNER)
      Bruckner::Model::State(name),
#endif // !defined(SYSTEMOC_ENABLE_MAESTRO) || !defined(MAESTRO_ENABLE_BRUCKNER)
      stateName(name)
  {
    assert(!name.empty());
#if defined(SYSTEMOC_ENABLE_MAESTRO) && defined(MAESTRO_ENABLE_BRUCKNER)
    dynamic_cast<Bruckner::Model::Hierarchical *>(this)->parent = sParent;
#endif //defined(SYSTEMOC_ENABLE_MAESTRO) && defined(MAESTRO_ENABLE_BRUCKNER)
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
