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

#include "ANDStateImpl.hpp"
#include "FiringFSM.hpp"

namespace smoc { namespace Detail { namespace FSM {

  ANDStateImpl::ANDStateImpl(std::string const &name)
    : StateImpl(name)
  {}
  
  void ANDStateImpl::add(StateImpl *part) {
    StateImpl::add(part);
  }
  
  void ANDStateImpl::getInitialState(
      ProdState &p, Marking const &m) const
  {
    for(C::const_iterator s = c.begin(); s != c.end(); ++s) {
      (*s)->getInitialState(p, m);
    }
  }

  #ifdef FSM_FINALIZE_BENCHMARK
  void ANDStateImpl::countStates(size_t& nLeaf, size_t& nAnd, size_t& nXor, size_t& nTrans) const {
    nAnd++;
    StateImpl::countStates(nLeaf, nAnd, nXor, nTrans);
  }
  #endif // FSM_FINALIZE_BENCHMARK

  StateImpl const *ANDStateImpl::getTopState(
      MultiState const &d, bool isSrcState) const
  {
    for(MultiState::const_iterator s = d.begin();
        s != d.end(); ++s)
    {
      if(!isAncestor(*s)) {
        assert(getParent());
        return getParent()->getTopState(d, false);
      }
      else if(*s == this) {
        isSrcState = true;
      }
    }
  
    if(isSrcState)
      return this;

    // if we come here, user tried to cross a partition
    // boundary without leaving the AND state

    throw FiringFSM::ModelingError("smoc_and_state: Can't create inter-partition transitions");
  }
  
  void intrusive_ptr_add_ref(ANDStateImpl *p)
    { intrusive_ptr_add_ref(static_cast<BaseStateImpl *>(p)); }
  
  void intrusive_ptr_release(ANDStateImpl *p)
    { intrusive_ptr_release(static_cast<BaseStateImpl *>(p)); }

} } } // namespace smoc::Detail::FSM