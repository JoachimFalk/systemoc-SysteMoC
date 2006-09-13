// vim: set sw=2 ts=8:
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <smoc_firing_rules.hpp>
#include <smoc_node_types.hpp>

#include <map>
#include <set>

#include <hscd_tdsim_TraceLog.hpp>

#ifdef ENABLE_SYSTEMC_VPC
# include <systemcvpc/hscd_vpc_Director.h>
#endif //ENABLE_SYSTEMC_VPC

#include <cosupport/oneof.hpp>

using namespace CoSupport;

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

smoc_firing_types::resolved_state_ty *smoc_firing_state_ref::finalise(smoc_root_node *actor) const {
  assert( rs != NULL && fr != NULL );
  fr->finalise(actor);
  return rs;
}

/*

bool smoc_firing_state_ref::tryExecute() {
  bool retval;
#ifdef SYSTEMOC_DEBUG
  std::cerr << "<tryExecute for "
            << fr->getActor()->myModule()->name() << ">" << std::endl;
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
#ifdef SYSTEMOC_DEBUG
  std::cerr << "smoc_firing_state::smoc_firing_state(...) this == " << this << std::endl;
#endif
}
smoc_firing_state::smoc_firing_state(const smoc_transition &t)
  :sc_object(sc_gen_unique_name("smoc_firing_state")) {
  this->operator = (t);
#ifdef SYSTEMOC_DEBUG
  std::cerr << "smoc_firing_state::smoc_firing_state(...) this == " << this << std::endl;
#endif
}
smoc_firing_state::smoc_firing_state()
  :sc_object(sc_gen_unique_name("smoc_firing_state")) {
#ifdef SYSTEMOC_DEBUG
  std::cerr << "smoc_firing_state::smoc_firing_state(...) this == " << this << std::endl;
#endif
}
smoc_firing_state::smoc_firing_state(const this_type &x)
  :sc_object(sc_gen_unique_name("smoc_firing_state")) {
  *this = x;
#ifdef SYSTEMOC_DEBUG
  std::cerr << "smoc_firing_state::smoc_firing_state(...) this == " << this << std::endl;
#endif
}

smoc_firing_state &smoc_firing_state::operator = (const this_type &rhs) {
  assert(rhs.rs != NULL && rhs.fr != NULL ||
         rhs.rs == NULL && rhs.fr == NULL);
#ifdef SYSTEMOC_DEBUG
  std::cerr << "smoc_firing_state::mkCopy(" << &rhs << ") this == " << this << std::endl;
#endif
  if (&rhs != this) {
    // remove old transition of state
    clearTransition();
    if (rhs.rs != NULL) {
      if (rs != NULL) {
        *rs = *rhs.rs;
        rhs.fr->unify(fr);
      } else {
        rs = new resolved_state_ty(*rhs.rs);
        rhs.fr->addRef(this);
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
#ifdef SYSTEMOC_DEBUG
  std::cerr << "smoc_firing_state::~smoc_firing_state() this == " << this << std::endl;
#endif
}

smoc_firing_rules::smoc_firing_rules(const smoc_firing_state_ref *s)
  : actor(NULL) {
  assert(s != NULL && s->rs != NULL);
  addRef(s);
  states.push_back(s->rs);
}

void smoc_firing_rules::_addRef(
    const smoc_firing_state_ref *s,
    const smoc_firing_rules     *p) {
  assert(s != NULL);
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

void smoc_firing_rules::unify( smoc_firing_rules *fr ) {
  if ( this != fr ) {
    smoc_firing_rules *src, *dest;
    
    if ( fr->references.size() < references.size() ) {
      dest = fr; src = this;
    } else {
      dest = this; src = fr;
    }
    for ( references_ty::iterator iter = src->references.begin();
          iter != src->references.end();
          ++iter )
      dest->_addRef(*iter, src);
    src->references.clear();
    dest->states.splice(
      dest->states.begin(), src->states );
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
#ifdef SYSTEMOC_DEBUG
  std::cerr << "~smoc_firing_rules() this == " << this << std::endl;
#endif
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
    resolved_state_ty **rs, smoc_root_node *actor) {
#ifdef SYSTEMOC_DEBUG
  std::cerr << "  <transition actor=\""
            << actor->myModule()->name() << "\">"
            << std::endl;
#endif
#ifdef SYSTEMOC_TRACE
  TraceLog.traceStartTryExecute(actor->myModule()->name()); //
#endif
  Expr::evalTo<Expr::CommSetup>(guard);
  
  assert( isType<NILTYPE>(f) ||
          isType<smoc_func_diverge>(f) ||
          isType<smoc_func_branch>(f) ||
          isType<smoc_func_call>(f) );
  if ( isType<smoc_func_diverge>(f) ) {
    // FIXME: this must only be used internally
    const smoc_firing_state &ns = static_cast<smoc_func_diverge &>(f)();
    *rs = ns.rs;
  } else if ( isType<smoc_func_branch>(f) ) {
    // FIXME: this must only be used internally
    const smoc_firing_state &ns = static_cast<smoc_func_branch &>(f)();
    statelist_ty::const_iterator iter = sl.begin();
    
#ifndef NDEBUG
    // check that ns is in sl
    while ( iter != sl.end() && (*iter) != ns.rs )
      ++iter;
    assert( iter != sl.end() );
#endif
    *rs = ns.rs;
  } else {
#ifdef SYSTEMOC_TRACE
    TraceLog.traceStartActor(actor->myModule()->name()); //
#endif
    // FIXME: we assume calls will only be used by leaf actors
    if ( isType<smoc_func_call>(f) ) {
      smoc_func_call &fc = f;
      
#ifdef SYSTEMOC_DEBUG
      std::cerr << "    <call actor=\""
                << actor->myModule()->name()
                << " func=\"" << fc.getFuncName()
                << "\">"
                << std::endl;
#endif
      
#ifdef SYSTEMOC_TRACE
      TraceLog.traceStartFunction(fc.getFuncName()); //
#endif
#ifdef ENABLE_SYSTEMC_VPC
      actor->vpc_event_dii.reset();

      actor->vpc_event_lat = new smoc_ref_event();
      SystemC_VPC::EventPair p(&actor->vpc_event_dii, actor->vpc_event_lat);

      SystemC_VPC::Director::getInstance().  	
	compute( actor->myModule()->name(), fc.getFuncName(), p);
#endif //ENABLE_SYSTEMC_VPC
      fc();
#ifdef SYSTEMOC_TRACE
      TraceLog.traceEndFunction(fc.getFuncName());  //
#endif
      
      assert( sl.size() == 1 );
      
#ifdef ENABLE_SYSTEMC_VPC
      *rs = actor->commstate.rs;
      // save guard and next state to later execute communication
      actor->nextState.rs = sl.front();
      actor->_guard       =  &guard;
      
      // actor->ports_setup = _ctx.ports_setup;
      // _ctx.ports_setup.clear();
# ifdef SYSTEMOC_DEBUG
      std::cerr << "      <communication type=\"defered\"/>" << std::endl;
# endif
#else
      *rs = sl.front();
#endif // ENABLE_SYSTEMC_VPC

#ifdef SYSTEMOC_DEBUG
      std::cerr << "    </call>"<< std::endl;
#endif
    } else {
      assert( sl.size() == 1 );
      *rs = sl.front();
    }
#ifdef SYSTEMOC_TRACE
    TraceLog.traceEndActor(actor->myModule()->name()); //
#endif
  }
  
#ifdef ENABLE_SYSTEMC_VPC
  if (!isType<smoc_func_call>(f))
    Expr::evalTo<Expr::CommExec>(guard, NULL);
#else
  Expr::evalTo<Expr::CommExec>(guard);
#endif

/*
  for ( smoc_port_list::iterator iter =  _ctx.ports_setup.begin();
        iter != _ctx.ports_setup.end();
        ++iter )
    (*iter)->commExec();
 */
#ifdef SYSTEMOC_TRACE
  TraceLog.traceEndTryExecute(actor->myModule()->name()); //
#endif
#ifdef SYSTEMOC_DEBUG
  std::cerr << "  </transition>"<< std::endl;
#endif
}

void smoc_firing_types::transition_ty::finalise(smoc_root_node *a) {
  assert(actor == NULL && a != NULL);
  actor = a;
  smoc_activation_pattern::finalise();
}

void smoc_firing_types::resolved_state_ty::finalise(smoc_root_node *a) {
  for ( transitionlist_ty::iterator titer = tl.begin();
        titer != tl.end();
        ++titer )
    titer->finalise(a);
}

void smoc_firing_types::transition_ty::dump(std::ostream &out) const {
  out << "transition(" << this << ", ap == ";
  smoc_event_and_list::dump(out);
  out   << ", status == " << smoc_activation_pattern::getStatus() << ")";
}
