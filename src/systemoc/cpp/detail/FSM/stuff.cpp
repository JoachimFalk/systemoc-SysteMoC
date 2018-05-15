// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
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

#include <algorithm>
#include <map>
#include <set>
#include <list>
#include <stdexcept>
#include <time.h>

#include <CoSupport/compatibility-glue/nullptr.h>

#include <CoSupport/SmartPtr/RefCountObject.hpp>
#include <CoSupport/DataTypes/oneof.hpp>
#include <CoSupport/String/Concat.hpp>
#include <CoSupport/Math/flog2.hpp>
#include <CoSupport/String/convert.hpp>

#include <systemoc/smoc_config.h>

#include <systemoc/detail/smoc_func_call.hpp>
#include <smoc/smoc_guard.hpp>

#include <smoc/smoc_actor.hpp>
#include <smoc/smoc_graph.hpp>
#include <smoc/detail/TraceLog.hpp>
#include <smoc/detail/DebugOStream.hpp>

#include "smoc_firing_rules_impl.hpp"
#include "FiringFSM.hpp"
#include "RuntimeFiringRule.hpp"
#include "../SimulationContext.hpp"

#ifdef SYSTEMOC_ENABLE_HOOKING
# include <boost/regex.hpp> 
# include "../TransitionHook.hpp"
#endif // SYSTEMOC_ENABLE_HOOKING

#ifdef MAESTRO_ENABLE_POLYPHONIC
# include <Maestro/PolyphoniC/ActionOnThreadVisitor.hpp>
#endif // MAESTRO_ENABLE_POLYPHONIC

using CoSupport::String::Concat;
using CoSupport::String::asStr;

namespace smoc { namespace Detail { namespace FSM {

using namespace CoSupport::DataTypes;

template<class C> inline bool single(const C& c) {
  if(c.empty()) return false;
  return ++c.begin() == c.end();
}

class TransitionActionNameVisitor {
public:
  typedef std::string result_type;
public:
  result_type operator()(smoc_action const &f) const {
    std::ostringstream str;
    
    for (smoc_action::const_iterator i = f.begin(); i != f.end(); ++i) {
      if (i != f.begin())
        str << ";";
      str << i->getFuncName() << "(";
      for (ParamInfoList::const_iterator pIter = i->getParams().begin();
           pIter != i->getParams().end();
           ++pIter) {
        if (pIter != i->getParams().begin())
          str << ",";
        str << pIter->value;
      }
      str << ")";
    }
    return str.str();
  }
};

class Action_HasWaitVisitor {
public:
  typedef bool result_type;
public:
  result_type operator()(const smoc_action &f) const {
    for (smoc_action::const_iterator i = f.begin(); i != f.end(); ++i) {
      std::string name = i->getFuncName();
      if ( name.find("simulateTime") != std::string::npos) {
        return true;
      }
    }
    return false;
  }
};

StateImpl::StateImpl(const std::string &name)
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

void StateImpl::add(StateImpl* state) {
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

const std::string& StateImpl::getName() const
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
void StateImpl::countStates(size_t& nLeaf, size_t& nAnd, size_t& nXor, size_t& nTrans) const {
  for(C::const_iterator s = c.begin(); s != c.end(); ++s) {
    (*s)->countStates(nLeaf, nAnd, nXor, nTrans);
  }
  BaseStateImpl::countStates(nLeaf, nAnd, nXor, nTrans);
}
#endif // FSM_FINALIZE_BENCHMARK

StateImpl* StateImpl::select(
    const std::string& name)
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


void StateImpl::setParent(StateImpl* v) {
  assert(v);
  if(parent && v != parent) {
    assert(!"smoc_hierarchical_state: Parent already set");
  }
  parent = v;
}

StateImpl* StateImpl::getParent() const {
  return parent;
}

bool StateImpl::isAncestor(const StateImpl* s) const {
  assert(s);

  if(s == this)
    return true;


  if(s->bits > bits)
    return (code == (s->code >> (s->bits - bits)));
  
  return false;
}
  
void StateImpl::mark(Marking& m) const {
  bool& mm = m[this];
  if(!mm) {
    mm = true;
    if(parent) parent->mark(m);
  }
}

bool StateImpl::isMarked(const Marking& m) const {
  Marking::const_iterator iter = m.find(this);
  return (iter == m.end()) ? false : iter->second;
}

void StateImpl::finalise(ExpandedTransitionList& etl) {
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<StateImpl::finalise name=\"" << getName() << "\">"
         << std::endl << smoc::Detail::Indent::Up;
  }
#endif // SYSTEMOC_DEBUG

#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "Code: " << code << "; Bits: " << bits << std::endl;
  }
#endif // SYSTEMOC_DEBUG

  if(!c.empty()) {
    size_t cs = c.size();
    size_t cb = CoSupport::Math::flog2c(static_cast<uint32_t>(cs));
    
#ifdef SYSTEMOC_DEBUG
    if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
      smoc::Detail::outDbg << "#C: " << cs << " -> CB: " << cb << std::endl;
    }
#endif // SYSTEMOC_DEBUG

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

#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</StateImpl::finalise>" << std::endl;
  }
#endif // SYSTEMOC_DEBUG
}

void StateImpl::expandTransition(
    ExpandedTransitionList &etl,
    StateImpl const        *srcState,
    CondMultiState const   &conditions,
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
  { intrusive_ptr_add_ref(static_cast<BaseStateImpl*>(p)); }

void intrusive_ptr_release(StateImpl *p)
  { intrusive_ptr_release(static_cast<BaseStateImpl*>(p)); }

FiringStateImpl::FiringStateImpl(const std::string& name)
  : StateImpl(name.empty()
      ? sc_core::sc_gen_unique_name("q", false)
      : name)
{}

void FiringStateImpl::getInitialState(
    ProdState& p, const Marking& m) const
{
//  smoc::Detail::outDbg << "FiringStateImpl::getInitialState(p,m) this == " << this << std::endl;
//  ScopedIndent s0(smoc::Detail::outDbg);

  p.insert(this);
}

#ifdef FSM_FINALIZE_BENCHMARK
void FiringStateImpl::countStates(size_t& nLeaf, size_t& nAnd, size_t& nXor, size_t& nTrans) const {
  nLeaf++;
  StateImpl::countStates(nLeaf, nAnd, nXor, nTrans);
}
#endif // FSM_FINALIZE_BENCHMARK

const StateImpl* FiringStateImpl::getTopState(
    const MultiState& d,
    bool isSrcState) const
{
  for(MultiState::const_iterator s = d.begin();
      s != d.end(); ++s)
  {
    if(*s != this) {
      assert(getParent());
      return getParent()->getTopState(d, false);
    }
  }

  return this;
}

void intrusive_ptr_add_ref(FiringStateImpl *p)
  { intrusive_ptr_add_ref(static_cast<BaseStateImpl*>(p)); }

void intrusive_ptr_release(FiringStateImpl *p)
  { intrusive_ptr_release(static_cast<BaseStateImpl*>(p)); }



XORStateImpl::XORStateImpl(const std::string& name)
  : StateImpl(name),
    init(0)
{}

void XORStateImpl::add(StateImpl* state, bool i) {
  StateImpl::add(state); 
  if(i) init = state;
}

void XORStateImpl::finalise(ExpandedTransitionList& etl) {
  if(!init)
    throw FiringFSM::ModelingError("smoc_xor_state: Must specify initial state");
  StateImpl::finalise(etl);
}

#ifdef FSM_FINALIZE_BENCHMARK
void XORStateImpl::countStates(size_t& nLeaf, size_t& nAnd, size_t& nXor, size_t& nTrans) const {
  nXor++;
  StateImpl::countStates(nLeaf, nAnd, nXor, nTrans);
}
#endif // FSM_FINALIZE_BENCHMARK

void XORStateImpl::getInitialState(
    ProdState& p, const Marking& m) const
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

const StateImpl* XORStateImpl::getTopState(
    const MultiState& d,
    bool isSrcState) const
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
  { intrusive_ptr_add_ref(static_cast<BaseStateImpl*>(p)); }

void intrusive_ptr_release(XORStateImpl *p)
  { intrusive_ptr_release(static_cast<BaseStateImpl*>(p)); }



ANDStateImpl::ANDStateImpl(const std::string& name)
  : StateImpl(name)
{}

void ANDStateImpl::add(StateImpl* part) {
  StateImpl::add(part);
}

void ANDStateImpl::getInitialState(
    ProdState& p, const Marking& m) const
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

const StateImpl* ANDStateImpl::getTopState(
    const MultiState& d,
    bool isSrcState) const
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
  { intrusive_ptr_add_ref(static_cast<BaseStateImpl*>(p)); }

void intrusive_ptr_release(ANDStateImpl *p)
  { intrusive_ptr_release(static_cast<BaseStateImpl*>(p)); }




JunctionStateImpl::JunctionStateImpl()
  : BaseStateImpl() {}

void JunctionStateImpl::expandTransition(
    ExpandedTransitionList &etl,
    StateImpl const        *srcState,
    CondMultiState const   &conditions,
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
  { intrusive_ptr_add_ref(static_cast<BaseStateImpl*>(p)); }

void intrusive_ptr_release(JunctionStateImpl *p)
  { intrusive_ptr_release(static_cast<BaseStateImpl*>(p)); }



MultiStateImpl::MultiStateImpl()
  : BaseStateImpl() {}

void MultiStateImpl::finalise(ExpandedTransitionList& etl) {
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
    StateImpl const        *srcState,
    CondMultiState const   &conditions,
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

void MultiStateImpl::addState(StateImpl* s) {
//  smoc::Detail::outDbg << "MultiStateImpl::addState(s) this == " << this << std::endl; 
//  ScopedIndent s0(smoc::Detail::outDbg);

//  smoc::Detail::outDbg << "state: " << s << std::endl;
  
  getFiringFSM()->unify(s->getFiringFSM());

  sassert(states.insert(s).second);
}

void MultiStateImpl::addCondState(StateImpl* s, bool neg) {
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
