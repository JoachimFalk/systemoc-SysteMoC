// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2012 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#include "XORStateImpl.hpp"
#include "FiringFSM.hpp"

namespace smoc { namespace Detail { namespace FSM {

  XORStateImpl::XORStateImpl(std::string const &name)
    : StateImpl(name), init(0)
    {}

  void XORStateImpl::add(StateImpl *state, bool i) {
    StateImpl::add(state);
    if (i)
      init = state;
  }

  void XORStateImpl::finalise(ExpandedTransitionList &etl) {
    if (!init)
      throw FiringFSM::ModelingError("smoc_xor_state: Must specify initial state");
    StateImpl::finalise(etl);
  }

#ifdef FSM_FINALIZE_BENCHMARK
  void XORStateImpl::countStates(size_t &nLeaf, size_t &nAnd, size_t &nXor, size_t &nTrans) const {
    nXor++;
    StateImpl::countStates(nLeaf, nAnd, nXor, nTrans);
  }
#endif // FSM_FINALIZE_BENCHMARK

  void XORStateImpl::getInitialState(
      ProdState &p, Marking const &m) const
  {
//  smoc::Detail::outDbg << "XORStateImpl::getInitialState(p,m) this == " << this << std::endl;
//  ScopedIndent s0(smoc::Detail::outDbg);

    StateImpl* t = 0;

    for(C::const_iterator s = c.begin(); s != c.end(); ++s) {
      if((*s)->isMarked(m)) {
        if(t)
          throw FiringFSM::ModelingError("smoc_xor_state: Must specify single initial state");
        t = *s;
      }
    }

    if(!t) t = init;
    assert(t);
    t->getInitialState(p, m);
  }
  
  StateImpl const *XORStateImpl::getTopState(
      MultiState const &d, bool isSrcState) const
  {
    for(MultiState::const_iterator s = d.begin();
        s != d.end(); ++s)
    {
      if(!isAncestor(*s)) {
        assert(getParent());
        return getParent()->getTopState(d, false);
      }
    }

    return this;
  }
  
  void intrusive_ptr_add_ref(XORStateImpl *p)
    { intrusive_ptr_add_ref(static_cast<BaseStateImpl *>(p)); }
  
  void intrusive_ptr_release(XORStateImpl *p)
    { intrusive_ptr_release(static_cast<BaseStateImpl *>(p)); }

} } } // namespace smoc::Detail::FSM
