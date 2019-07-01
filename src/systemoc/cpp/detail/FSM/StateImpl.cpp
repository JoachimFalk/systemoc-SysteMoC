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

#include "StateImpl.hpp"
#include "XORStateImpl.hpp"
#include "FiringFSM.hpp"

#include <CoSupport/Math/flog2.hpp>

namespace smoc { namespace Detail { namespace FSM {

  StateImpl::StateImpl(std::string const &name)
    : BaseStateImpl()
    , name(name)
    , parent(nullptr)
    , code(0), bits(1)
  {
    if(name.find(FiringFSM::HIERARCHY_SEPARATOR) != std::string::npos)
      assert(!"smoc_hierarchical_state: Invalid state name");
    if(name.find(FiringFSM::PRODSTATE_SEPARATOR) != std::string::npos)
      assert(!"smoc_hierarchical_state: Invalid state name");
  }

  StateImpl::~StateImpl() {
    for(C::const_iterator s = c.begin(); s != c.end(); ++s) {
      assert((*s)->getFiringFSM() == this->getFiringFSM());
      delete *s;
    }
  }

  void StateImpl::add(StateImpl *state) {
    getFiringFSM()->unify(state->getFiringFSM());
    getFiringFSM()->delState(state);
    c.push_back(state);
    state->setParent(this);
  }
  
  void StateImpl::setFiringFSM(FiringFSM *fsm) {
    BaseStateImpl::setFiringFSM(fsm);
    for(C::const_iterator s = c.begin(); s != c.end(); ++s) {
      (*s)->setFiringFSM(fsm);
    }
  }

  std::string const &StateImpl::getName() const
    { return name; }

  std::string StateImpl::getHierarchicalName() const {
    if(!parent || parent == getFiringFSM()->top) {
      return name;
    }
    std::string parentName = parent->getHierarchicalName();
    if (name.empty())
      return parentName;
    else if (parentName.empty())
      return name;
    else
      return parentName +
        FiringFSM::HIERARCHY_SEPARATOR +
        name;
  }

#ifdef FSM_FINALIZE_BENCHMARK
  void StateImpl::countStates(size_t &nLeaf, size_t &nAnd, size_t &nXor, size_t &nTrans) const {
    for (C::const_iterator s = c.begin(); s != c.end(); ++s) {
      (*s)->countStates(nLeaf, nAnd, nXor, nTrans);
    }
    BaseStateImpl::countStates(nLeaf, nAnd, nXor, nTrans);
  }
#endif // FSM_FINALIZE_BENCHMARK

  StateImpl *StateImpl::select(
      std::string const &name)
  {
    size_t pos = name.find(FiringFSM::HIERARCHY_SEPARATOR);
    std::string top = name.substr(0, pos);

    if(top.empty()) {
      if(pos == std::string::npos)
        return this;
      else
        assert(!"smoc_hierarchical_state: Invalid hierarchical name");
    }

    for(C::iterator s = c.begin(); s != c.end(); ++s) {
      if((*s)->getName() == top) {
        if(pos == std::string::npos)
          return (*s);
        else
          return (*s)->select(name.substr(pos + 1));
      }
    }

    assert(!"smoc_hierarchical_state:: Invalid hierarchical name");
    return nullptr;
  }
  
  
  void StateImpl::setParent(StateImpl *v) {
    assert(v);
    if(parent && v != parent) {
      assert(!"smoc_hierarchical_state: Parent already set");
    }
    parent = v;
  }

  StateImpl *StateImpl::getParent() const {
    return parent;
  }

  bool StateImpl::isAncestor(StateImpl const *s) const {
    assert(s);

    if(s == this)
      return true;


    if(s->bits > bits)
      return (code == (s->code >> (s->bits - bits)));

    return false;
  }
    
  void StateImpl::mark(Marking &m) const {
    bool& mm = m[this];
    if(!mm) {
      mm = true;
      if(parent) parent->mark(m);
    }
  }

  bool StateImpl::isMarked(Marking const &m) const {
    Marking::const_iterator iter = m.find(this);
    return (iter == m.end()) ? false : iter->second;
  }

  void StateImpl::finalise(ExpandedTransitionList &etl) {
#ifdef SYSTEMOC_ENABLE_DEBUG
    if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
      smoc::Detail::outDbg << "<StateImpl::finalise name=\"" << getName() << "\">"
           << std::endl << smoc::Detail::Indent::Up;
    }
#endif // SYSTEMOC_ENABLE_DEBUG

#ifdef SYSTEMOC_ENABLE_DEBUG
    if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
      smoc::Detail::outDbg << "Code: " << code << "; Bits: " << bits << std::endl;
    }
#endif // SYSTEMOC_ENABLE_DEBUG

    if(!c.empty()) {
      size_t cs = c.size();
      size_t cb = CoSupport::Math::flog2c(static_cast<uint32_t>(cs));

#ifdef SYSTEMOC_ENABLE_DEBUG
      if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
        smoc::Detail::outDbg << "#C: " << cs << " -> CB: " << cb << std::endl;
      }
#endif // SYSTEMOC_ENABLE_DEBUG

      uint64_t cc = code << cb;

      cb += bits;
      assert(cb < 64);

      for(C::const_iterator s = c.begin(); s != c.end(); ++s) {
        (*s)->code = cc;
        (*s)->bits = cb;
        (*s)->finalise(etl);
        ++cc;
      }
    }
  
    CondMultiState noConditions;

    for(PartialTransitionList::const_iterator pt = ptl.begin();
        pt != ptl.end(); ++pt)
    {
      assert(pt->getDestState());
      pt->getDestState()->expandTransition(
          etl, this, noConditions, *pt);
    }

#ifdef SYSTEMOC_ENABLE_DEBUG
    if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
      smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</StateImpl::finalise>" << std::endl;
    }
#endif // SYSTEMOC_ENABLE_DEBUG
  }

  void StateImpl::expandTransition(
      ExpandedTransitionList &etl,
      StateImpl        const *srcState,
      CondMultiState   const &conditions,
      smoc_firing_rule const &accFiringRule) const
  {
//  smoc::Detail::outDbg << "StateImpl::expandTransition(etl,t) this == " << this << std::endl;
//  ScopedIndent s0(smoc::Detail::outDbg);

    MultiState dest;
    dest.insert(this);

    etl.push_back(
        ExpandedTransition(
          srcState, conditions,
          getFiringFSM()->acquireFiringRule(accFiringRule),
          dest));
  }
  
  void intrusive_ptr_add_ref(StateImpl *p)
    { intrusive_ptr_add_ref(static_cast<BaseStateImpl *>(p)); }
  
  void intrusive_ptr_release(StateImpl *p)
    { intrusive_ptr_release(static_cast<BaseStateImpl *>(p)); }

} } } // namespace smoc::Detail::FSM
