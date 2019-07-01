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

#include "JunctionStateImpl.hpp"

#include "FiringFSM.hpp"

namespace smoc { namespace Detail { namespace FSM {

  JunctionStateImpl::JunctionStateImpl()
    : BaseStateImpl() {}
  
  void JunctionStateImpl::expandTransition(
      ExpandedTransitionList &etl,
      StateImpl        const *srcState,
      CondMultiState   const &conditions,
      smoc_firing_rule const &accFiringRule) const
  {
//  smoc::Detail::outDbg << "JunctionStateImpl::expandTransition(etl,t) this == " << this << std::endl;
//  ScopedIndent s0(smoc::Detail::outDbg);
    if (ptl.empty())
      throw FiringFSM::ModelingError("smoc_junction_state: Must specify at least one transition");
  
    for (PartialTransitionList::const_iterator pt = ptl.begin();
         pt != ptl.end();
         ++pt)
    {
      assert(pt->getDestState());
      pt->getDestState()->expandTransition(
          etl, srcState, conditions,
          smoc_firing_rule(
              accFiringRule.getGuard() && pt->getGuard(),
              merge(accFiringRule.getAction(), pt->getAction())));
    }
  }
  
  void intrusive_ptr_add_ref(JunctionStateImpl *p)
    { intrusive_ptr_add_ref(static_cast<BaseStateImpl *>(p)); }
  
  void intrusive_ptr_release(JunctionStateImpl *p)
    { intrusive_ptr_release(static_cast<BaseStateImpl *>(p)); }
  
} } } // namespace smoc::Detail::FSM
