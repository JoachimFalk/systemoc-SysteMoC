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

#include "BaseStateImpl.hpp"

#include "FiringFSM.hpp"

namespace smoc { namespace Detail { namespace FSM {

  BaseStateImpl::BaseStateImpl()
    : fsm(new FiringFSM()) {
//  std::cerr << "BaseStateImpl::BaseStateImpl() this == "
//            << this << std::endl;
    fsm->addState(this);
  }

  BaseStateImpl::~BaseStateImpl() {
//  std::cerr << "BaseStateImpl::~BaseStateImpl() this == "
//            << this << std::endl;
  }

  FiringFSM *BaseStateImpl::getFiringFSM() const
    { return fsm; }

  void BaseStateImpl::setFiringFSM(FiringFSM *f)
    { fsm = f; }

  void BaseStateImpl::addTransition(const smoc_transition_list &stl) {
    for(smoc_transition_list::const_iterator st = stl.begin();
        st != stl.end();
        ++st) {
      PartialTransition pt(
          st->getGuard(),
          st->getAction(),
          st->getDestState().getImpl());
      ptl.push_back(pt);
      assert(pt.getDestState());
      fsm->unify(pt.getDestState()->getFiringFSM());
    }
  }

  void BaseStateImpl::clearTransition()
    { ptl.clear(); }

#ifdef FSM_FINALIZE_BENCHMARK
  void BaseStateImpl::countStates(size_t& nLeaf, size_t& nAnd, size_t& nXOR, size_t& nTrans) const {
    nTrans += ptl.size();
  }
#endif // FSM_FINALIZE_BENCHMARK

  void intrusive_ptr_add_ref(BaseStateImpl *p)
    { p->getFiringFSM()->addRef(); }

  void intrusive_ptr_release(BaseStateImpl *p)
    { if(p->getFiringFSM()->delRef()) delete p->getFiringFSM(); }

} } } // namespace smoc::Detail::FSM
