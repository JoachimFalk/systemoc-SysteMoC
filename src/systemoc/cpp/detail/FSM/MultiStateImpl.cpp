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

#include "MultiStateImpl.hpp"
#include "StateImpl.hpp"
#include "FiringFSM.hpp"

namespace smoc { namespace Detail { namespace FSM {

  template<class C>
  inline bool single(C const &c) {
    if (c.empty())
      return false;
    return ++c.begin() == c.end();
  }

  MultiStateImpl::MultiStateImpl()
    : BaseStateImpl() {}

  void MultiStateImpl::finalise(ExpandedTransitionList &etl) {
//  smoc::Detail::outDbg << "MultiStateImpl::finalise(etl) this == " << this << std::endl;
//  ScopedIndent s0(smoc::Detail::outDbg);

    // target state if no transitions
    if (ptl.empty())
      return;

    // at least one outgoing transition --> single src state
    if (!single(states))
      throw FiringFSM::ModelingError("smoc_multi_state: Source multi-state must specify exactly one source state!");

    for (PartialTransitionList::const_iterator pt = ptl.begin();
         pt != ptl.end();
         ++pt) {
      assert(pt->getDestState());
      pt->getDestState()->expandTransition(
          etl, *states.begin(), condStates, *pt);
    }
  }

  void MultiStateImpl::expandTransition(
      ExpandedTransitionList &etl,
      StateImpl        const *srcState,
      CondMultiState   const &conditions,
      smoc_firing_rule const &accFiringRule) const
  {
//  smoc::Detail::outDbg << "MultiStateImpl::expandTransition(etl,t) this == " << this << std::endl;
//  ScopedIndent s0(smoc::Detail::outDbg);
    if (states.empty())
      throw FiringFSM::ModelingError("smoc_multi_state: Destination multi-state must specify at least one target state!");
    if (!condStates.empty())
      throw FiringFSM::ModelingError("smoc_multi_state: Destination multi-state must not specify any condition states!");

    etl.push_back(
        ExpandedTransition(
          srcState, conditions,
          getFiringFSM()->acquireFiringRule(accFiringRule),
          states));
  }

  void MultiStateImpl::addState(StateImpl *s) {
//  smoc::Detail::outDbg << "MultiStateImpl::addState(s) this == " << this << std::endl; 
//  ScopedIndent s0(smoc::Detail::outDbg);

//  smoc::Detail::outDbg << "state: " << s << std::endl;

    getFiringFSM()->unify(s->getFiringFSM());

    sassert(states.insert(s).second);
  }
  
  void MultiStateImpl::addCondState(StateImpl *s, bool neg) {
//  smoc::Detail::outDbg << "MultiStateImpl::addCondState(s) this == " << this << std::endl;
//  ScopedIndent s0(smoc::Detail::outDbg);

//  smoc::Detail::outDbg << "state: " << s << std::endl;

    getFiringFSM()->unify(s->getFiringFSM());

    sassert(condStates.insert(std::make_pair(s, neg)).second);
  }

  void intrusive_ptr_add_ref(MultiStateImpl *p)
    { intrusive_ptr_add_ref(static_cast<BaseStateImpl*>(p)); }
  
  void intrusive_ptr_release(MultiStateImpl *p)
    { intrusive_ptr_release(static_cast<BaseStateImpl*>(p)); }

} } } // namespace smoc::Detail::FSM
