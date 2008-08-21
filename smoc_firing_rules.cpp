//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
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

#ifndef VERBOSE_LEVEL_SMOC_FIRING_RULES
#define VERBOSE_LEVEL_SMOC_FIRING_RULES 0
///100: Actor invocation
#endif

#include <systemoc/smoc_config.h>

#include <systemoc/smoc_firing_rules.hpp>
#include <systemoc/smoc_node_types.hpp>
#include <systemoc/smoc_graph_type.hpp>

#include <typeinfo>

#include <map>
#include <set>

#include <systemoc/hscd_tdsim_TraceLog.hpp>

#ifdef SYSTEMOC_ENABLE_VPC
# include <systemoc/MultiHopEvent.hpp>
# include <systemcvpc/hscd_vpc_Director.h>
#endif //SYSTEMOC_ENABLE_VPC

#include <CoSupport/DataTypes/oneof.hpp>

#include <algorithm>

//using CoSupport::DataTypes::isType;

using namespace CoSupport::DataTypes;

smoc_firing_state_ref::smoc_firing_state_ref(
    const smoc_firing_state_ref &rhs)
  : rs(&rhs.getResolvedState()), fr(NULL) {
  rhs.fr->addRef(this);
  assert(rhs.rs == rs && rhs.fr == fr);
}

smoc_firing_state_ref::resolved_state_ty &
smoc_firing_state_ref::getResolvedState() const {
  if (rs == NULL) {
    assert(fr == NULL);
    rs                   = new resolved_state_ty();
    smoc_firing_rules *x = new smoc_firing_rules(this);
    assert(fr == x);
  }
  return *rs;
}

//smoc_firing_types::resolved_state_ty *smoc_firing_state_ref::finalise(smoc_root_node *actor) const {
smoc_firing_types::resolved_state_ty *smoc_firing_state::finalise(smoc_root_node *actor) const {
#ifdef SYSTEMOC_DEBUG
//std::cerr << "smoc_firing_state_ref::finalise() begin, this == " << this << std::endl;
  std::cerr << "smoc_firing_state::finalise() begin, name == " << this->name() << std::endl;
#endif
  assert( rs != NULL && fr != NULL );
  fr->finalise(actor);
#ifdef SYSTEMOC_DEBUG
//std::cerr << "smoc_firing_state_ref::finalise() end, this == " << this << std::endl;
  std::cerr << "smoc_firing_state::finalise() end, name == " << this->name() << std::endl;
#endif
  return rs;
}

/*

bool smoc_firing_state_ref::tryExecute() {
  bool retval;
#ifdef SYSTEMOC_DEBUG
  std::cerr << "<tryExecute for "
            << fr->getActor()->name() << ">" << std::endl;
#endif
  retval = rs->tryExecute(&rs, fr->getActor());
#ifdef SYSTEMOC_DEBUG
  std::cerr << "</tryExecute>" << std::endl;
#endif
  return retval;
}

*/

void smoc_firing_state_ref::dump( std::ostream &o ) const {
  o << "state (" << this << "): ";
  for ( transitionlist_ty::const_iterator titer = rs->tl.begin();
        titer != rs->tl.end();
        ++titer )
    o << *titer << std::endl;
}

smoc_firing_state_ref::~smoc_firing_state_ref() {
  if ( fr )
    fr->delRef(this);
}

smoc_firing_state::smoc_firing_state(const smoc_transition_list &tl)
  :sc_object(sc_gen_unique_name("smoc_firing_state")) {
  this->operator = (tl);
//#ifdef SYSTEMOC_DEBUG
//  std::cerr << "smoc_firing_state::smoc_firing_state(...) this == " << this << std::endl;
//#endif
}
smoc_firing_state::smoc_firing_state(const smoc_transition &t)
  :sc_object(sc_gen_unique_name("smoc_firing_state")) {
  this->operator = (t);
//#ifdef SYSTEMOC_DEBUG
//  std::cerr << "smoc_firing_state::smoc_firing_state(...) this == " << this << std::endl;
//#endif
}
smoc_firing_state::smoc_firing_state()
  :sc_object(sc_gen_unique_name("smoc_firing_state")) {
//#ifdef SYSTEMOC_DEBUG
//  std::cerr << "smoc_firing_state::smoc_firing_state(...) this == " << this << std::endl;
//#endif
}
smoc_firing_state::smoc_firing_state(const this_type &x)
  :sc_object(sc_gen_unique_name("smoc_firing_state")) {
  *this = x;
//#ifdef SYSTEMOC_DEBUG
//  std::cerr << "smoc_firing_state::smoc_firing_state(...) this == " << this << std::endl;
//#endif
}

smoc_firing_state &smoc_firing_state::operator = (const this_type &rhs) {
  assert(rhs.rs != NULL && rhs.fr != NULL ||
         rhs.rs == NULL && rhs.fr == NULL);
//#ifdef SYSTEMOC_DEBUG
//  std::cerr << "smoc_firing_state::mkCopy(" << &rhs << ") this == " << this << std::endl;
//#endif
  if (&rhs != this) {
    // remove old transition of state
    clearTransition();
    if (rhs.rs != NULL) {
      if (rs != NULL) {
        *rs = *rhs.rs;
        rhs.fr->unify(fr);
      } else {
        rs = new resolved_state_ty(*rhs.rs);
        rhs.fr->addState(this);
      }
    }
  }
  return *this;
}

smoc_firing_state &smoc_firing_state::operator = (const smoc_transition_list &tl) {
  // remove old transition of state
  clearTransition();
  // add transitions
  addTransition(tl);
  return *this;
}

smoc_firing_state &smoc_firing_state::operator = (const smoc_transition &t)
  { return *this = static_cast<smoc_transition_list>(t); }

void smoc_firing_state::addTransition(
    const smoc_transition_list &tl) {
  getResolvedState().addTransition(this, tl);
}

void smoc_firing_state::clearTransition() {
  if (rs != NULL)
    // remove old transition of state
    rs->clearTransitions();
}

smoc_firing_state::~smoc_firing_state() {
//#ifdef SYSTEMOC_DEBUG
//  std::cerr << "smoc_firing_state::~smoc_firing_state() this == " << this << std::endl;
//#endif
}

void smoc_firing_rules::_addRef(
    const smoc_firing_state_ref *s,
    const smoc_firing_rules     *p) {
  assert(s != NULL && s->rs != NULL);
  // Resolved state must be included in state list
  assert(std::find(states.begin(), states.end(), s->rs) != states.end());
  sassert(references.insert(s).second && s->fr == p);
  s->fr = this;
}

void smoc_firing_rules::delRef(
    const smoc_firing_state_ref *s) {
  sassert(references.erase(s) == 1 && s->fr == this);
  s->fr = NULL; s->rs = NULL;
  if ( references.empty() )
    delete this;
}

void smoc_firing_rules::addState(
    const smoc_firing_state_ref *s) {
  assert(s != NULL && s->rs != NULL);
  // Must not be included previously
  assert(std::find(states.begin(), states.end(), s->rs) == states.end());
  states.push_back(s->rs);
  addRef(s);
}

void smoc_firing_rules::unify( smoc_firing_rules *fr ) {
  if ( this != fr ) {
    smoc_firing_rules *src, *dest;
    
    if ( fr->references.size() < references.size() ) {
      dest = fr; src = this;
    } else {
      dest = this; src = fr;
    }
    dest->states.splice(
      dest->states.begin(), src->states );
    for ( references_ty::iterator iter = src->references.begin();
          iter != src->references.end();
          ++iter )
      dest->_addRef(*iter, src);
    src->references.clear();
    delete src;
  }
}

void smoc_firing_rules::finalise( smoc_root_node *actor_ ) {
  // assert( unresolved_states.empty() );
  assert( actor == NULL );
  actor = actor_;

  for (statelist_ty::iterator iter = states.begin();
       iter != states.end();
       ++iter)
    (*iter)->finalise(actor_);
}

smoc_firing_rules::~smoc_firing_rules() {
//#ifdef SYSTEMOC_DEBUG
//  std::cerr << "~smoc_firing_rules() this == " << this << std::endl;
//#endif
  for ( statelist_ty::iterator iter = states.begin();
        iter != states.end();
        ++iter )
    delete *iter;
}


smoc_firing_types::transition_ty::transition_ty(
    smoc_firing_state_ref *s, const smoc_transition &t)
  : smoc_activation_pattern(t.getActivationPattern()),
    f (t.getInterfaceAction().f),
    actor(NULL) {
  assert(s->fr != NULL && s->rs != NULL);
  assert((isType<smoc_func_call>(t.ia.f)    && t.ia.sl.size() == 1) ||
         (isType<smoc_sr_func_pair>(t.ia.f) && t.ia.sl.size() == 1) ||
         (isType<smoc_func_diverge>(t.ia.f) && t.ia.sl.size() == 0) ||
   (isType<NILTYPE>(t.ia.f)           && t.ia.sl.size() == 1));
  for ( smoc_firing_state_list::const_iterator siter = t.ia.sl.begin();
        siter != t.ia.sl.end();
        ++siter ) {
    sl.push_front(&siter->getResolvedState());
    s->fr->unify(siter->fr);
  }
}

void
smoc_firing_types::resolved_state_ty::clearTransitions()
  { tl.clear(); }

void
smoc_firing_types::resolved_state_ty::addTransition(
    smoc_firing_state_ref *r, const smoc_transition_list &tl_ ) {
  for ( smoc_transition_list::const_iterator titer = tl_.begin();
        titer != tl_.end();
        ++titer )
    tl.push_back(transition_ty(r, *titer));
}

void smoc_firing_types::transition_ty::execute(
    resolved_state_ty **rs, smoc_root_node *actor, int mode) {
  enum {
    MODE_DIISTART,
    MODE_DIIEND,
    MODE_GRAPH
  } execMode;
  
  if (dynamic_cast<smoc_graph *>(actor) == NULL) {
    execMode =
#ifdef SYSTEMOC_ENABLE_VPC  
      *rs != actor->commstate.rs
        ? MODE_DIISTART
        : MODE_DIIEND;
#else
      MODE_DIISTART;
#endif
  } else {
#ifdef SYSTEMOC_ENABLE_VPC  
    assert(*rs != actor->commstate.rs);
#endif
    execMode = MODE_GRAPH;
  }
  
#ifdef SYSTEMOC_DEBUG
  static const char *execModeName[] = { "diiStart", "diiEnd", "graph" };
  
  std::cerr << "  <transition "
      "actor=\"" << actor->name() << "\" "
      "mode=\"" << execModeName[execMode] << "\""
    ">" << std::endl;
#endif
//#ifdef SYSTEMOC_TRACE
//  TraceLog.traceStartTryExecute(name);
//#endif
  
  assert(isType<NILTYPE>(f) ||
         isType<smoc_func_diverge>(f) ||
         isType<smoc_func_branch>(f) ||
         isType<smoc_func_call>(f) ||
         isType<smoc_sr_func_pair>(f));
  
#ifdef SYSTEMOC_TRACE
  if (execMode != MODE_GRAPH)
    // leaf actor
    TraceLog.traceStartActor(actor, execMode == MODE_DIISTART ? "s" : "e");
#endif
  
#if !defined(NDEBUG) || defined(SYSTEMOC_TRACE)
  Expr::evalTo<Expr::CommSetup>(guard);
#endif
  
  resolved_state_ty *nextState;
  
  if (isType<smoc_func_diverge>(f)) {
    // FIXME: this must only be used internally
#if defined(SYSTEMOC_DEBUG)  || (VERBOSE_LEVEL_SMOC_FIRING_RULES >= 100)
    std::cerr << "    <smoc_func_diverge func=\"???\">" << std::endl;
#endif
    const smoc_firing_state &ns = static_cast<smoc_func_diverge &>(f)();
#if defined(SYSTEMOC_DEBUG)  || (VERBOSE_LEVEL_SMOC_FIRING_RULES >= 100)
    std::cerr << "    </smoc_func_diverge>" << std::endl;
#endif
    nextState = ns.rs;
  } else if (isType<smoc_func_branch>(f)) {
    // FIXME: this must only be used internally
#if defined(SYSTEMOC_DEBUG)  || (VERBOSE_LEVEL_SMOC_FIRING_RULES >= 100)
    std::cerr << "    <smoc_func_branch func=\"???\">" << std::endl;
#endif
    const smoc_firing_state &ns = static_cast<smoc_func_branch &>(f)();
#if defined(SYSTEMOC_DEBUG)  || (VERBOSE_LEVEL_SMOC_FIRING_RULES >= 100)
    std::cerr << "    </smoc_func_branch>" << std::endl;
#endif
    statelist_ty::const_iterator iter = sl.begin();
    
#ifndef NDEBUG
    // check that ns is in sl
    while (iter != sl.end() && (*iter) != ns.rs)
      ++iter;
    assert(iter != sl.end());
#endif
    nextState = ns.rs;
  } else if (isType<smoc_func_call>(f)) {
    smoc_func_call &fc = f;
    
#ifdef SYSTEMOC_TRACE
    TraceLog.traceStartFunction(fc.getFuncName()); //
#endif
#if defined(SYSTEMOC_DEBUG)  || (VERBOSE_LEVEL_SMOC_FIRING_RULES >= 100)
    std::cerr << "    <smoc_func_call func=\"" << fc.getFuncName() << "\">" << std::endl;
#endif
    fc();

#if defined(SYSTEMOC_DEBUG)  || (VERBOSE_LEVEL_SMOC_FIRING_RULES >= 100)
    std::cerr << "    </smoc_func_call>" << std::endl;
#endif
#ifdef SYSTEMOC_TRACE
    TraceLog.traceEndFunction(fc.getFuncName());  //
#endif
    
    assert(sl.size() == 1);
    nextState = sl.front();
  } else if (isType<smoc_sr_func_pair>(f)) {
    // SR GO & TICK calls:
    smoc_sr_func_pair &fc = f;
    
#ifdef SYSTEMOC_TRACE
    TraceLog.traceStartFunction(fc.go.getFuncName());
#endif
#ifdef SYSTEMOC_DEBUG
    if(mode & GO)
      std::cerr << "    <smoc_sr_func_pair go=\"" << fc.go.getFuncName() << "\">" << std::endl;
    if(mode & TICK)
      std::cerr << "    <smoc_sr_func_pair tick=\"" << fc.tick.getFuncName() << "\">" << std::endl;
#endif
    if(mode & GO)   fc.go();
    if(mode & TICK) fc.tick();
#ifdef SYSTEMOC_DEBUG
    std::cerr << "    </smoc_sr_func_pair>" << std::endl;
#endif
#ifdef SYSTEMOC_TRACE
    TraceLog.traceEndFunction(fc.go.getFuncName());
#endif
    
    assert(sl.size() == 1);
    nextState = sl.front();
  } else {
    // a transition without action (no CALL statement)
    assert(isType<NILTYPE>(f));
    assert(sl.size() == 1);
    nextState = sl.front();
  }
  
#if !defined(NDEBUG)
  Expr::evalTo<Expr::CommReset>(guard);
#endif
  
#ifdef SYSTEMOC_ENABLE_VPC
  if (execMode == MODE_DIISTART /*&& (mode&GO)*/) {
    actor->vpc_event_dii.reset();
    
    actor->vpc_event_lat = new smoc_ref_event();
    SystemC_VPC::EventPair p(&actor->vpc_event_dii, actor->vpc_event_lat);
    

    // FIXME: We schould do that (collecting input channels) in finalise()
    MultiHopEvent * hopEvent = new MultiHopEvent;
    Expr::port_commit_map pm;
    Expr::evalTo<Expr::CommitCount>(guard, pm);
    for(Expr::port_commit_map::const_iterator i = pm.begin();
        i != pm.end();
        ++i){

      smoc_root_port * port = i->first;
      smoc_root_chan * chan =
        dynamic_cast<smoc_root_chan *>( port->get_interface());

      if( chan != NULL && port->isInput() ){
        hopEvent->addInputChannel(chan, i->second);
        //std::cerr << "port: " << port->name() <<" commitCount: " << i->second
        //          << " chan=" << chan->name() << std::endl;
      } else if( chan != NULL && !port->isInput() ){
        hopEvent->addOutputChannel(chan, i->second);
      } else if(  NULL != dynamic_cast<smoc_entry_kind *>( port->get_interface() ) ) {
         smoc_root_chan * chan = dynamic_cast<smoc_entry_kind *>( port->get_interface() )->getRootChan();
         assert(!port->isInput());
         hopEvent->addOutputChannel(chan, i->second);
         //std::cerr << "entry: " << port->name() <<" commitCount: " << i->second
         //         << " chan=" << chan->name() << std::endl;

      }
    }
    hopEvent->setTaskEvents(p);

    // new FastLink interface
    if(mode & GO) hopEvent->compute( vpcLink );
    else if(mode & TICK){
      assert( isType<smoc_sr_func_pair>(f) );
      smoc_sr_func_pair &fp = f;
      hopEvent->compute( fp.tickLink );
    }
    // save guard and nextState to later execute communication
    actor->_guard       =  &guard;
    actor->nextState.rs = nextState;
    // Insert magic commstate
    nextState           = actor->commstate.rs;
  } else {
    Expr::evalTo<Expr::CommExec>(guard, NULL);
  }
#else // !SYSTEMOC_ENABLE_VPC
  Expr::evalTo<Expr::CommExec>(guard);
#endif // !SYSTEMOC_ENABLE_VPC
  
#ifdef SYSTEMOC_TRACE
  if (execMode != MODE_GRAPH)
    TraceLog.traceEndActor(actor);
#endif
 
  *rs = nextState;
 
//#ifdef SYSTEMOC_TRACE
//  TraceLog.traceEndTryExecute(name);
//#endif
#ifdef SYSTEMOC_DEBUG
  std::cerr << "  </transition>"<< std::endl;
#endif
}

void smoc_firing_types::transition_ty::finalise(smoc_root_node *a) {
  assert(actor == NULL && a != NULL);
  actor = a;
  smoc_activation_pattern::finalise();

#ifdef SYSTEMOC_ENABLE_VPC
  if (dynamic_cast<smoc_actor *>(actor) != NULL) {
    const char *name = actor->name();
    if (isType<smoc_func_call>(f)) {
      smoc_func_call &fc = f;
      vpcLink = new SystemC_VPC::FastLink(SystemC_VPC::Director::getInstance().
        getFastLink(name, fc.getFuncName()));
    } else if (isType<smoc_sr_func_pair>(f)) {
      smoc_sr_func_pair &fp = f;
      vpcLink = new SystemC_VPC::FastLink(SystemC_VPC::Director::getInstance().
        getFastLink(name, fp.go.getFuncName()));
      fp.tickLink = new SystemC_VPC::FastLink(
        SystemC_VPC::Director::getInstance().
        getFastLink(name, fp.tick.getFuncName()));
    } else {
      vpcLink = new SystemC_VPC::FastLink(SystemC_VPC::Director::getInstance().
        getFastLink(name, "???"));
    }
  }
#endif //SYSTEMOC_ENABLE_VPC
}

void smoc_firing_types::resolved_state_ty::finalise(smoc_root_node *a) {
  for ( transitionlist_ty::iterator titer = tl.begin();
        titer != tl.end();
        ++titer )
    titer->finalise(a);
}

#ifdef SYSTEMOC_DEBUG
void smoc_firing_types::transition_ty::dump(std::ostream &out) const {
  out << "transition(" << this << ", ap == ";
  smoc_event_and_list::dump(out);
  out   << ", status == " << smoc_activation_pattern::getStatus().toSymbol()
        << ")";
}
#endif
