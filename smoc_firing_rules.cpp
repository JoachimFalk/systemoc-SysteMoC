// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2006 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
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

#include <CoSupport/SmartPtr/RefCountObject.hpp>
#include <CoSupport/DataTypes/oneof.hpp>

#include <systemoc/smoc_config.h>

#include <systemoc/smoc_node_types.hpp>
#include <systemoc/smoc_graph_type.hpp>
#include <systemoc/hscd_tdsim_TraceLog.hpp>
#include <systemoc/smoc_ngx_sync.hpp>
#include <systemoc/smoc_firing_rules.hpp>
#include <systemoc/detail/smoc_firing_rules_impl.hpp>

using namespace CoSupport::DataTypes;
using namespace SysteMoC::NGXSync;

PartialTransition::PartialTransition(const smoc_transition &t)
  : ap(t.getActivationPattern()),
    f(t.getInterfaceAction().getAction()),
    dest(t.getInterfaceAction().getDestState().getImpl())
  {}

PartialTransition::PartialTransition(
    const smoc_activation_pattern& ap,
    const smoc_action& f,
    FiringStateBaseImpl* dest)
  : ap(ap),
    f(f),
    dest(dest)
{}

const smoc_activation_pattern &PartialTransition::getActivationPattern() const
  { return ap; }

const smoc_action& PartialTransition::getAction() const
  { return f; }

FiringStateBaseImpl* PartialTransition::getDestState() const
  { return dest; }


ExpandedTransition::ExpandedTransition(
    const PartialTransition &t, FiringStateImpl *dest)
  : smoc_activation_pattern(t.getActivationPattern()),
    f(t.getAction()),
    dest(dest),
    actor(0)
  {}

#ifdef SYSTEMOC_DEBUG
Expr::Detail::ActivationStatus ExpandedTransition::getStatus() const {
  std::cerr << "ExpandedTransition::getStatus: " << *this << std::endl;
  return smoc_activation_pattern::getStatus();
}
#endif

bool ExpandedTransition::isEnabled() const
  { return getStatus() == Expr::Detail::ENABLED(); }

smoc_root_node &ExpandedTransition::getActor()
  { assert(actor != NULL); return *actor; }

void ExpandedTransition::execute(int mode) {
  enum {
    MODE_DIISTART,
    MODE_DIIEND,
    MODE_GRAPH
  } execMode;
  
  if(dynamic_cast<smoc_graph_base*>(actor) == NULL) {
    execMode =
#ifdef SYSTEMOC_ENABLE_VPC
      actor->getCurrentState() != actor->getCommState()
        ? MODE_DIISTART
        : MODE_DIIEND;
#else
    MODE_DIISTART;
#endif
  }
  else {
#ifdef SYSTEMOC_ENABLE_VPC
    assert(actor->getCurrentState() != actor->getCommState());
#endif
    execMode = MODE_GRAPH;
  }

#ifdef SYSTEMOC_DEBUG
  static const char *execModeName[] = { "diiStart", "diiEnd", "graph" };

  std::cerr << "  <transition actor=\"" << actor->name()
            << "\" mode=\"" << execModeName[execMode]
            << "\">" << std::endl;
#endif

#ifdef SYSTEMOC_TRACE
  if(execMode != MODE_GRAPH)
    TraceLog.traceStartActor(actor, execMode == MODE_DIISTART ? "s" : "e");
#endif

#if defined(SYSTEMOC_ENABLE_DEBUG) || defined(SYSTEMOC_TRACE)
  Expr::evalTo<Expr::CommSetup>(guard);
#endif

  // only smoc_func_diverge may set nextState to something
  // different than dest here...
  FiringStateImpl* nextState =
    boost::apply_visitor(ActionVisitor(dest, mode), f);

#if defined(SYSTEMOC_ENABLE_DEBUG)
  Expr::evalTo<Expr::CommReset>(guard);
#endif

#ifdef SYSTEMOC_ENABLE_VPC
  if(execMode == MODE_DIISTART /*&& (mode&GO)*/) {
    actor->diiEvent->reset();
    smoc_ref_event_p latEvent(new smoc_ref_event());
    
    SystemC_VPC::EventPair p(actor->diiEvent.get(), latEvent.get());

    // new FastLink interface
    if(mode & GO) {
      vpcLink->compute(p);
    }
    else if(mode & TICK) {
      smoc_sr_func_pair* fp = boost::get<smoc_sr_func_pair>(&f);
      assert(fp);
      fp->tickLink->compute(p);
    }

    // save nextState to later execute communication
    actor->setNextState(nextState);

    // insert magic commstate
    nextState = actor->getCommState();
    Expr::evalTo<Expr::CommExec>(guard, actor->diiEvent, latEvent);

    // This covers the case that the executed transition does not
    // contain an output port. Therefore, the latEvent is not added
    // to a LatencyQueue and would be deleted immediately after
    // the latEvent smartptr is destroyed when this scope is left.
    if(!*latEvent) {
      // latency event not signaled
      struct _: public smoc_event_listener {
        smoc_ref_event_p  latEvent;
        smoc_root_node   *actor;
        
        void signaled(smoc_event_waiter *_e) {
# ifdef SYSTEMOC_TRACE
  //      const char *name = actor->name();
          
          TraceLog.traceStartActor(actor, "l");
# endif
# ifdef SYSTEMOC_DEBUG
          std::cerr << "smoc_root_node::_communicate::_::signaled(...)"
                    << std::endl;
# endif
          assert(_e == &*latEvent);
          assert(*_e);
          latEvent = NULL;
# ifdef SYSTEMOC_TRACE
          TraceLog.traceEndActor(actor);
# endif
          return;
        }
        void eventDestroyed(smoc_event_waiter *_e) {
# ifdef SYSTEMOC_DEBUG
          std::cerr << "smoc_root_node::_communicate::_:: eventDestroyed(...)"
                    << std::endl;
# endif
          delete this;
        }
        
        _(const smoc_ref_event_p &latEvent, smoc_root_node *actor)
          : latEvent(latEvent), actor(actor) {};
        
        virtual ~_() {}
      };
      latEvent->addListener(new _(latEvent, actor));
    }
    else {
# ifdef SYSTEMOC_TRACE
      TraceLog.traceStartActor(this, "l");
      TaceLog.traceEndActor(this);
# endif
    }
  }
  else {
    Expr::evalTo<Expr::CommExec>(guard, NULL, NULL);
  }
#else // SYSTEMOC_ENABLE_VPC
  Expr::evalTo<Expr::CommExec>(guard);
#endif // SYSTEMOC_ENABLE_VPC

#ifdef SYSTEMOC_TRACE
  if(execMode != MODE_GRAPH)
    TraceLog.traceEndActor(actor);
#endif

  actor->setCurrentState(nextState);

#ifdef SYSTEMOC_DEBUG
  std::cerr << "  </transition>"<< std::endl;
#endif
}

void ExpandedTransition::finalise(smoc_root_node *a) {
  assert(actor == NULL);
  assert(a != NULL);

  actor = a;
  smoc_activation_pattern::finalise();    

  //ActionUnifier au;
  //boost::apply_visitor(au, f);
  //f = au.getAction();

#ifdef SYSTEMOC_ENABLE_VPC
  if(dynamic_cast<smoc_actor *>(actor) != NULL) {
    vpcLink = boost::apply_visitor(
        VPCLinkVisitor(actor->name()), f);
  }
#endif //SYSTEMOC_ENABLE_VPC
}

FiringStateImpl* ExpandedTransition::getDestState() const
  { return dest; }

const smoc_action& ExpandedTransition::getAction() const
  { return f; }


FiringFSMImpl::FiringFSMImpl()
  : use_count_(0), actor(0) {
  //std::cerr << "FiringFSMImpl::FiringFSMImpl() this == " << this << std::endl;
}

FiringFSMImpl::~FiringFSMImpl() {
  //std::cerr << "FiringFSMImpl::~FiringFSMImpl() this == " << this << std::endl;
  assert(use_count_ == 0);  
  
  for(FiringStateBaseImplSet::iterator iter = states.begin();
      iter != states.end();
      ++iter)
  {
    assert((*iter)->getFiringFSM() == this);
    delete *iter;
  }
}

smoc_root_node* FiringFSMImpl::getActor() const
  { assert(actor != NULL); return actor; }

const FiringStateImplSet& FiringFSMImpl::getLeafStates() const
  { return leafStates; }

void FiringFSMImpl::finalise(smoc_root_node *_actor) {
  assert(actor == NULL);
  actor = _actor;

  for(FiringStateBaseImplSet::iterator iter = states.begin();
      iter != states.end();
      ++iter)
  {
    (*iter)->finalise(actor);
  }
}

void FiringFSMImpl::addState(FiringStateBaseImpl *state) {
  assert(state->getFiringFSM() == this);
  sassert(states.insert(state).second);
}

void FiringFSMImpl::delState(FiringStateBaseImpl *state) {
  assert(state->getFiringFSM() == this);
  sassert(states.erase(state) == 1);
}

void FiringFSMImpl::addRef() {
  ++use_count_;
  //std::cerr << "FiringFSMImpl::add_ref() this == " << this
  //          << "; use_count: " << use_count_ << std::endl;
}

bool FiringFSMImpl::delRef() {
  assert(use_count_); --use_count_;
  //std::cerr << "FiringFSMImpl::delRef() this == " << this
  //          << "; use_count: " << use_count_ << std::endl;
  return use_count_ == 0;
}

void FiringFSMImpl::unify(this_type *fr) {
  if(this != fr) {
    //std::cerr << "FiringFSMImpl::unify() this == " << this
    //          << "; other: " << fr << std::endl;
    
    // patch firingFSM of all states owned by fr
    for(FiringStateBaseImplSet::iterator iter = fr->states.begin();
        iter != fr->states.end();
        ++iter)
    {
      sassert(states.insert(*iter).second);
      assert((*iter)->getFiringFSM() == fr);
      (*iter)->setFiringFSM(this);
    }
    
    //std::cerr << " own use_count: " << use_count_
    //          << "; other use_count: " << fr->use_count_
    //          << "; # merged states: " << states.size()
    //          << std::endl;

    use_count_ += fr->use_count_;
    fr->use_count_ = 0;
    fr->states.clear();    
    
    delete fr;
  }
}

void FiringFSMImpl::addLeafState(FiringStateImpl *state) {
  assert(state->getFiringFSM() == this);
  sassert(leafStates.insert(state).second);
}

FiringStateBaseImpl::FiringStateBaseImpl()
  : firingFSM(new FiringFSMImpl()) {
  //std::cerr << "FiringStateBaseImpl::FiringStateBaseImpl() this == "
  //          << this << std::endl;
  firingFSM->addState(this);
}

FiringStateBaseImpl::FiringStateBaseImpl(const PFiringStateBaseImpl &s)
  : firingFSM(s->firingFSM) {
  //std::cerr << "FiringStateBaseImpl::FiringStateBaseImpl(" << s.get() << ") this == "
  //          << this << std::endl;
  firingFSM->addState(this);
}

FiringStateBaseImpl::~FiringStateBaseImpl() {
  //std::cerr << "FiringStateBaseImpl::~FiringStateBaseImpl() this == "
  //          << this << std::endl;
}

FiringFSMImpl *FiringStateBaseImpl::getFiringFSM() const
  { return firingFSM; }

void FiringStateBaseImpl::setFiringFSM(FiringFSMImpl *fsm)
  { firingFSM = fsm; }

void FiringStateBaseImpl::addTransition(const smoc_transition_list &tl_) {
  for(smoc_transition_list::const_iterator iter = tl_.begin();
      iter != tl_.end();
      ++iter)
  {
    addTransition(PartialTransition(*iter));
  }
}
  
void FiringStateBaseImpl::addTransition(const PartialTransitionList& pl) {
  for(PartialTransitionList::const_iterator iter = pl.begin();
      iter != pl.end();
      ++iter)
  {
    addTransition(*iter);
  }
}

void FiringStateBaseImpl::addTransition(const PartialTransition& t) {
  tl.push_back(t);
  if(t.getDestState())
    firingFSM->unify(t.getDestState()->firingFSM);
}

void FiringStateBaseImpl::clearTransition()
  { tl.clear(); }

FiringStateBaseImpl& FiringStateBaseImpl::operator=(const this_type& s) {
  if(&s != this) {
    clearTransition();
    tl = s.tl;
    s.firingFSM->unify(firingFSM);
  }
  return *this;
}

FiringStateBaseImpl& FiringStateBaseImpl::operator=(const smoc_transition_list& tl) {
  clearTransition();
  addTransition(tl);
  return *this;
}

void intrusive_ptr_add_ref(FiringStateBaseImpl *p)
  { p->getFiringFSM()->addRef(); }

void intrusive_ptr_release(FiringStateBaseImpl *p)
  { if(p->getFiringFSM()->delRef()) delete p->getFiringFSM(); }





FiringStateImpl::FiringStateImpl()
  : FiringStateBaseImpl(),
    sc_object(sc_gen_unique_name("smoc_firing_state")) {
  idPool.regObj(this);
}

FiringStateImpl::FiringStateImpl(const PFiringStateImpl &s)
  : FiringStateBaseImpl(s),
    sc_object(sc_gen_unique_name("smoc_firing_state")) {
  idPool.regObj(this);
}

FiringStateImpl::~FiringStateImpl() {
  idPool.unregObj(this);
}

void FiringStateImpl::finalise(smoc_root_node *actor) {
  for(PartialTransitionList::const_iterator plIter = tl.begin();
      plIter != tl.end(); ++plIter)
  {
    if(plIter->getDestState()) {
      // may be unefficient if more than one partial state
      // exists with the same target state...
      PartialTransitionList pl;
      pl.push_back(*plIter);
      
      ExpandedTransitionList tmp =
        plIter->getDestState()->expandTransitions(pl);

      el.splice(el.end(), tmp);
    }
    else {
      // smoc_func_diverge has no dest state
      el.push_back(ExpandedTransition(*plIter, 0));
    }
  }
  
  for(ExpandedTransitionList::iterator elIter = el.begin();
      elIter != el.end(); ++elIter)
  {
    elIter->finalise(actor);
  }

  firingFSM->addLeafState(this);
}

ExpandedTransitionList FiringStateImpl::expandTransitions(
    const PartialTransitionList &pl)
{
  // Task: determine the target state of the partial transitions
  // (This instance is the target state of all partial
  // transitions which are given as an argument [stops recursion])
  ExpandedTransitionList ret;
  for(PartialTransitionList::const_iterator iter = pl.begin();
      iter != pl.end();
      ++iter)
  {
    ret.push_back(ExpandedTransition(*iter, this));
  }
  return ret;
}

FiringStateImpl& FiringStateImpl::operator=(const smoc_transition_list &tl)
  { FiringStateBaseImpl::operator=(tl); return *this; }

void intrusive_ptr_add_ref(FiringStateImpl *p)
  { intrusive_ptr_add_ref(static_cast<FiringStateBaseImpl*>(p)); }

void intrusive_ptr_release(FiringStateImpl *p)
  { intrusive_ptr_release(static_cast<FiringStateBaseImpl*>(p)); }




RefinedStateImpl::RefinedStateImpl(FiringStateBaseImpl *init)
  : FiringStateBaseImpl(), init(init) {
  add(init);
}

RefinedStateImpl::RefinedStateImpl(const PRefinedStateImpl &s)
  : FiringStateBaseImpl(s)
{}

RefinedStateImpl::~RefinedStateImpl() {
  for(FiringStateBaseImplSet::iterator iter = states.begin();
      iter != states.end();
      ++iter)
  {
    assert((*iter)->getFiringFSM() == firingFSM);
    delete *iter;
  }
}

void RefinedStateImpl::setFiringFSM(FiringFSMImpl *fsm) {
  for(FiringStateBaseImplSet::iterator iter = states.begin();
      iter != states.end();
      ++iter)
  {
    (*iter)->setFiringFSM(fsm);
  }
  FiringStateBaseImpl::setFiringFSM(fsm);
}

void RefinedStateImpl::add(FiringStateBaseImpl *state) {
  firingFSM->unify(state->getFiringFSM());
  firingFSM->delState(state);
  sassert(states.insert(state).second);
}

void RefinedStateImpl::finalise(smoc_root_node *actor) {
  // Add my transitions to each state's outgoing transition
  // list, then finalise it (recursive)
  for(FiringStateBaseImplSet::iterator iter = states.begin();
      iter != states.end();
      ++iter)
  {
    (*iter)->addTransition(tl);
    (*iter)->finalise(actor);
  }
}

ExpandedTransitionList
RefinedStateImpl::expandTransitions(const PartialTransitionList &pl) {
  // Task: determine the target state of the partial transitions
  // (The target state of all partial transitions which are given
  // as an argument is my inital state [may be recursive])
  return init->expandTransitions(pl);
}

RefinedStateImpl& RefinedStateImpl::operator=(const smoc_transition_list &tl)
  { FiringStateBaseImpl::operator=(tl); return *this; }

void intrusive_ptr_add_ref(RefinedStateImpl *p)
  { intrusive_ptr_add_ref(static_cast<FiringStateBaseImpl*>(p)); }

void intrusive_ptr_release(RefinedStateImpl *p)
  { intrusive_ptr_release(static_cast<FiringStateBaseImpl*>(p)); }


ConnectorStateImpl::ConnectorStateImpl()
  : FiringStateBaseImpl() {
}

ConnectorStateImpl::ConnectorStateImpl(const PConnectorStateImpl &s)
  : FiringStateBaseImpl(s) {
}

ConnectorStateImpl::~ConnectorStateImpl() {
}

void ConnectorStateImpl::finalise(smoc_root_node *actor) {

}

ExpandedTransitionList
ConnectorStateImpl::expandTransitions(const PartialTransitionList &pl) {
  // Task: determine the target state of the partial transitions

  ExpandedTransitionList ret;

  for(PartialTransitionList::const_iterator i = pl.begin();
      i != pl.end(); ++i)
  {
    for(PartialTransitionList::const_iterator j = tl.begin();
        j != tl.end(); ++j)
    {
      // FIXME: Currently, we assume that input/output requirements of both
      // transitions are disjoint, so we can just AND them together. In the
      // general case, however, we must accumulate the requirements for same
      // ports and add some kind of offset to the requests in the second
      // action.

      // create a new partial transition (i,j) and ask j.destState to
      // expand it...
      PartialTransition p(
          i->getActivationPattern().getExpr() && j->getActivationPattern().getExpr(),
          merge(i->getAction(), j->getAction()),
          j->getDestState());

      PartialTransitionList pl;
      pl.push_back(p);

      ExpandedTransitionList tmp =
        j->getDestState()->expandTransitions(pl);

      ret.splice(ret.end(), tmp);
    }
  }

  return ret;
}

ConnectorStateImpl& ConnectorStateImpl::operator=(const smoc_transition_list &tl)
  { FiringStateBaseImpl::operator=(tl); return *this; }

void intrusive_ptr_add_ref(ConnectorStateImpl *p)
  { intrusive_ptr_add_ref(static_cast<FiringStateBaseImpl*>(p)); }

void intrusive_ptr_release(ConnectorStateImpl *p)
  { intrusive_ptr_release(static_cast<FiringStateBaseImpl*>(p)); }



smoc_firing_state_base::smoc_firing_state_base(const SmartPtr &p)
  : FFType(p) {}

void smoc_firing_state_base::addTransition(const smoc_transition_list &tl)
  { getImpl()->addTransition(tl); }

void smoc_firing_state_base::clearTransition()
  { getImpl()->clearTransition(); }



smoc_firing_state::smoc_firing_state(const SmartPtr &p)
  : FFType(p) {}

smoc_firing_state::smoc_firing_state()
  : FFType(new FiringStateImpl()) {}

smoc_firing_state::smoc_firing_state(const this_type &s)
  : FFType(new FiringStateImpl(s.getImpl())) {}

smoc_firing_state::ImplType *smoc_firing_state::getImpl() const
  { return static_cast<ImplType *>(this->pImpl.get()); }

smoc_firing_state& smoc_firing_state::operator = (const this_type &t)
  { *getImpl() = *t.getImpl(); return *this; }

smoc_firing_state& smoc_firing_state::operator = (const smoc_transition_list &tl)
  { *getImpl() = tl; return *this; }



smoc_refined_state::smoc_refined_state(const SmartPtr &p)
  : FFType(p) {}

smoc_refined_state::smoc_refined_state(const smoc_firing_state_base &init)
  : FFType(new RefinedStateImpl(init.getImpl())) {}

smoc_refined_state::smoc_refined_state(const this_type &s)
  : FFType(new RefinedStateImpl(s.getImpl())) {}

smoc_refined_state::ImplType *smoc_refined_state::getImpl() const
  { return static_cast<ImplType *>(this->pImpl.get()); }

void smoc_refined_state::add(const smoc_firing_state_base &state)
  { getImpl()->add(state.getImpl()); }

smoc_refined_state& smoc_refined_state::operator = (const this_type &t)
  { *getImpl() = *t.getImpl(); return *this; }

smoc_refined_state& smoc_refined_state::operator = (const smoc_transition_list &tl)
  { *getImpl() = tl; return *this; }



smoc_connector_state::smoc_connector_state(const SmartPtr &p)
  : FFType(p) {}

smoc_connector_state::smoc_connector_state()
  : FFType(new ConnectorStateImpl()) {}

smoc_connector_state::smoc_connector_state(const this_type &s)
  : FFType(new ConnectorStateImpl(s.getImpl())) {}

smoc_connector_state::ImplType *smoc_connector_state::getImpl() const
  { return static_cast<ImplType *>(this->pImpl.get()); }

smoc_connector_state& smoc_connector_state::operator = (const this_type &t)
  { *getImpl() = *t.getImpl(); return *this; }

smoc_connector_state& smoc_connector_state::operator = (const smoc_transition_list &tl)
  { *getImpl() = tl; return *this; }
