// vim: set sw=2 ts=8:

#include <smoc_firing_rules.hpp>
#include <smoc_node_types.hpp>

#include <map>
#include <set>

#include <hscd_tdsim_TraceLog.hpp>

#include <systemcvpc/hscd_vpc_Director.h>

smoc_firing_state_ref::smoc_firing_state_ref()
  : rs(new resolved_state_ty()), fr(NULL) {
  smoc_firing_rules *x = new smoc_firing_rules(this);
  assert( x == fr );
}

smoc_firing_state_ref::smoc_firing_state_ref( const this_type &rhs )
  : rs(rhs.rs), fr(NULL) {
  assert( rhs.rs != NULL && rhs.fr != NULL );
  rhs.fr->addRef(this);
}

void smoc_firing_state_ref::mkCopy( const this_type &rhs ) {
  // remove old transition of state
  getResolvedState().clearTransitions();
  *rs = rhs.getResolvedState();
  fr->unify( rhs.fr );
}

void smoc_firing_state_ref::finalise( smoc_root_node *actor ) const {
  assert( rs != NULL && fr != NULL );
  fr->finalise(actor);
}

bool smoc_firing_state_ref::tryExecute() {
  bool retval;
#ifdef SYSTEMOC_DEBUG
  std::cout << "<tryExecute for "
            << fr->getActor()->myModule()->name() << ">" << std::endl;
#endif
  retval = rs->tryExecute(&rs, fr->getActor());
#ifdef SYSTEMOC_DEBUG
  std::cout << "</tryExecute>" << std::endl;
#endif
  return retval;
}

void smoc_firing_state_ref::findBlocked(smoc_root_port_bool_list &l) {
#ifdef SYSTEMOC_DEBUG
  std::cout << "<findBlocked for "
            << fr->getActor()->myModule()->name() << ">" << std::endl;
#endif
  rs->findBlocked(l, fr->getActor());
#ifdef SYSTEMOC_DEBUG
  std::cout << "</findBlocked>" << std::endl;
#endif
}

void smoc_firing_state_ref::dump( std::ostream &o ) const {
  o << "state (" << this << "): ";
  for ( transitionlist_ty::const_iterator titer = rs->tl.begin();
        titer != rs->tl.end();
        ++titer )
    o << *titer << std::endl;
}

smoc_firing_state &smoc_firing_state::operator = (const this_type &x) {
  if ( &x != this )
    mkCopy(x);
  return *this;
}

smoc_firing_state &smoc_firing_state::operator = (const smoc_transition_list &tl) {
  // remove old transition of state
  getResolvedState().clearTransitions();
  // add transitions
  getResolvedState().addTransition(this, tl);
  return *this;
}

smoc_firing_state &smoc_firing_state::operator = (const smoc_transition &t)
  { return *this = static_cast<smoc_transition_list>(t); }

void smoc_firing_state::addTransition( const smoc_transition_list &tl )
  { getResolvedState().addTransition(this, tl); }

void smoc_firing_rules::_addRef( smoc_firing_state_ref *s, smoc_firing_rules *p ) {
  assert( s != NULL );
  bool success = references.insert(s).second;
  assert( success && s->fr == p );
  s->fr = this;
}

void smoc_firing_rules::delRef( smoc_firing_state_ref *s ) {
  references_ty::iterator iter = references.find(s);
  
  assert( iter != references.end() && s->fr == this );
  references.erase( iter ); s->fr = NULL; s->rs = NULL;
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
}

smoc_firing_types::transition_ty::transition_ty(
    smoc_firing_state_ref *s, const smoc_transition &t ) {
  ap = t.getActivationPattern();
  f  = t.ia.f;
  assert( s->fr != NULL && s->rs != NULL );
  for ( smoc_firing_state_list::const_iterator siter = t.ia.sl.begin();
        siter != t.ia.sl.end();
        ++siter ) {
    if ( siter->rs != NULL ) {
      sl.push_front( siter->rs );
      s->fr->unify( siter->fr );
    }
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

bool
smoc_firing_types::resolved_state_ty::tryExecute(
    resolved_state_ty **rs, smoc_root_node *actor) {
  bool retval = false;
  
  for ( transitionlist_ty::iterator titer = tl.begin();
        titer != tl.end() && !retval;
        ++titer ) {
    assert( _ctx.ports_setup.empty() );
    retval |= titer->tryExecute(rs,actor);
    for ( smoc_port_list::iterator iter =  _ctx.ports_setup.begin();
          iter != _ctx.ports_setup.end();
          ++iter )
      (*iter)->reset();
    _ctx.ports_setup.clear();
  }
  return retval;
}

bool smoc_firing_types::transition_ty::tryExecute(
    resolved_state_ty **rs, smoc_root_node *actor) {
  bool canexec =
    knownSatisfiable().getStatus() == smoc_root_port_bool::IS_ENABLED;
  
  assert( isType<NILTYPE>(f) ||
          isType<smoc_func_diverge>(f) ||
          isType<smoc_func_branch>(f) ||
          isType<smoc_func_call>(f) );
  if ( canexec ) {
    TraceLog.traceStartTryExecute(actor->myModule()->name()); //
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
      TraceLog.traceStartActor(actor->myModule()->name()); //
      // FIXME: we assume calls will only be used by leaf actors
      if ( isType<smoc_func_call>(f) ) {
        smoc_func_call &fc = f;
        
#ifdef SYSTEMOC_DEBUG
        std::cout << "<call actor="<<actor->myModule()->name()
                  << " func="<< fc.getFuncName()
                  << ">"<< std::endl;
#endif
        
	TraceLog.traceStartFunction(fc.getFuncName()); //
        actor->vpc_event.reset();
        SystemC_VPC::Director::getInstance().
          getResource( actor->myModule()->name() ).
            compute( actor->myModule()->name(),
                     fc.getFuncName(),
                     &actor->vpc_event );
        fc();
	TraceLog.traceEndFunction(fc.getFuncName());  //
        
#ifdef SYSTEMOC_DEBUG
        std::cout << "</call>"<< std::endl;
#endif
        assert( sl.size() == 1 );
        
        actor->nextState.rs = sl.front();
        *rs = actor->commstate.rs;
      } else {
        assert( sl.size() == 1 );
        *rs = sl.front();
      }
      TraceLog.traceEndActor(actor->myModule()->name()); //
    }
    for ( smoc_port_list::iterator iter =  _ctx.ports_setup.begin();
          iter != _ctx.ports_setup.end();
          ++iter )
      (*iter)->commExec();
    TraceLog.traceEndTryExecute(actor->myModule()->name()); //
  }
  return canexec;
}

void smoc_firing_types::resolved_state_ty::findBlocked(
    smoc_root_port_bool_list &l, smoc_root_node *actor) {
  for ( transitionlist_ty::iterator titer = tl.begin();
        titer != tl.end();
        ++titer ) {
    titer->findBlocked(l, actor);
  }
}

void smoc_firing_types::transition_ty::findBlocked(
    smoc_root_port_bool_list &l, smoc_root_node *actor) {
  smoc_root_port_bool b      = knownSatisfiable();
  
  // std::cout << b << std::endl;
#ifdef SYSTEMOC_DEBUG
  std::cout << "  <transition status=" << b.getStatus() << "/>" << std::endl;
#endif
  if ( b.getStatus() != smoc_root_port_bool::IS_DISABLED ) {
    assert( b.getStatus() == smoc_root_port_bool::IS_BLOCKED );
    l.push_back(b);
  }
}

void smoc_firing_types::transition_ty::dump(std::ostream &out) const {
  out << "transition("
        << this << ","
        << "status="   << knownSatisfiable().getStatus() << ","
//        << "knownUnsatisfiable=" << knownUnsatisfiable() << ", "
        << "ap: "                << ap << ")";
}


