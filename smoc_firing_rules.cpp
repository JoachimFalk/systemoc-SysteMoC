// vim: set sw=2 ts=8:

#include <smoc_firing_rules.hpp>
#include <smoc_node_types.hpp>

#include <map>
#include <set>

#include <hscd_vpc_Director.h>
#include <hscd_tdsim_TraceLog.hpp>

smoc_firing_state &smoc_firing_state::operator = (const smoc_transition_list &tl) {
  if ( fr != NULL )
    fr->delRef(this);
  assert( fr == NULL && rs == NULL );
  rs = new resolved_state_ty();
  smoc_firing_rules *x = new smoc_firing_rules(this);
  assert( x == fr );
  for ( smoc_transition_list::const_iterator titer = tl.begin();
        titer != tl.end();
        ++titer ) {
    transition_ty &t = rs->addTransition();
    t.initTransition(fr,*titer);
  }
  fr->resolve();
  return *this;
}

smoc_firing_state &smoc_firing_state::operator = (const smoc_transition &t) {
  return *this = static_cast<smoc_transition_list>(t);
}

smoc_firing_state &smoc_firing_state::operator = (const this_type &x) {
  if ( &x != this ) {
    if ( fr != NULL )
      fr->delRef(this);
    assert( fr == NULL );
    rs = x.rs;
    if ( x.fr != NULL ) {
      x.fr->addRef(this);
      assert( fr == x.fr );
      fr->resolve();
    } else {
      assert( rs == NULL );
    }
  }
  return *this;
}

void smoc_firing_rules::_addRef( smoc_firing_state *s, smoc_firing_rules *p ) {
  assert( s != NULL );
  bool success = references.insert(s).second;
  assert( success && s->fr == p );
  s->fr = this;
}

void smoc_firing_rules::delRef( smoc_firing_state *s ) {
  references_ty::iterator iter = references.find(s);
  
  assert( iter != references.end() && s->fr == this );
  references.erase( iter ); s->fr = NULL; s->rs = NULL;
  if ( references.empty() )
    delete this;
}

void smoc_firing_rules::resolve() {
  for ( unresolved_states_ty::iterator iter = unresolved_states.begin();
        iter != unresolved_states.end();
        ) {
    references_ty::iterator riter = references.find(**iter);
    
    if ( riter != references.end() ) {
      /* found it => resolve it */
      **iter = (*riter)->rs;
      /* delete from unresolved list */
      iter = unresolved_states.erase(iter);
    } else
      iter++;
  }
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
    dest->resolved_states.splice(
      dest->resolved_states.begin(), src->resolved_states );
    dest->unresolved_states.splice(
      dest->unresolved_states.begin(), src->unresolved_states );
    delete src;
    dest->resolve();
  }
}

void smoc_firing_state::finalise( smoc_root_node *actor ) const {
  if ( fr == NULL )
    return;
  assert( fr != NULL );
  fr->finalise(actor);
}

void smoc_firing_rules::finalise( smoc_root_node *actor_ ) {
  assert( unresolved_states.empty() );
  assert( actor == NULL );
  actor = actor_;
}

smoc_port_list &smoc_firing_state::getPorts() const {
  assert( fr != NULL && fr->actor != NULL );
  return fr->actor->getPorts();
}

void smoc_firing_types::transition_ty::initTransition(
    smoc_firing_rules *fr,
    const smoc_transition &t ) {
  ap = t.getActivationPattern();
  f  = t.ia.f;
  for ( smoc_firing_state_list::const_iterator siter = t.ia.sl.begin();
        siter != t.ia.sl.end();
        ++siter ) {
    if ( !siter->isResolved() ) {
      // unresolved state
      sl.push_back(static_cast<smoc_firing_state *>(*siter));
      fr->addUnresolved(&sl.back());
    } else {
      // resolved state
      sl.push_back(static_cast<smoc_firing_state>(*siter).rs);
      fr->unify(static_cast<smoc_firing_state>(*siter).fr);
    }
  }
}

bool smoc_firing_state::tryExecute() {
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
  smoc_actor *a       = dynamic_cast<smoc_actor *>(actor);
  bool        canexec =
    (!a || a->vpc_event) &&
    knownSatisfiable().getStatus() == smoc_root_port_bool::IS_ENABLED;
  
  if ( canexec ) {
    TraceLog.traceStartTryExecute(actor->myModule()->name()); //
    if ( isType<smoc_func_diverge>(f) ) {
      // FIXME: this must only be used internally
      const smoc_firing_state &ns = static_cast<smoc_func_diverge &>(f)();
      *rs = &ns.getResolvedState();
    } else if ( isType<smoc_func_branch>(f) ) {
      // FIXME: this must only be used internally
      const smoc_firing_state &ns = static_cast<smoc_func_branch &>(f)();
      statelist_ty::const_iterator iter = sl.begin();
      
      // check that ns is in sl
      while ( iter != sl.end() && (*iter) != ns.rs )
        ++iter;
      assert( iter != sl.end() );
      *rs = &ns.getResolvedState();
    } else {
      TraceLog.traceStartActor(actor->myModule()->name()); //
      // FIXME: we assume calls will only be used by leaf actors
      if ( isType<smoc_func_call>(f) ) {
#ifdef SYSTEMOC_DEBUG
        std::cerr << "<call actor="<<actor->myModule()->name()
                  << " func="<< static_cast<smoc_func_call &>(f).getFuncName()
                  << ">"<< std::endl;
#endif
	TraceLog.traceStartFunction(static_cast<smoc_func_call &>(f).getFuncName()); //
        a->vpc_event.reset();
        SystemC_VPC::Director::getInstance().getResource( actor->myModule()->name() ).compute( 
            actor->myModule()->name(), static_cast<smoc_func_call &>(f).getFuncName()  ,&(a->vpc_event) );
        static_cast<smoc_func_call &>(f)();
	TraceLog.traceEndFunction(static_cast<smoc_func_call &>(f).getFuncName());  //

#ifdef SYSTEMOC_DEBUG
        std::cerr << "</call>"<< std::endl;
#endif

	TraceLog.traceEndActor(actor->myModule()->name()); //
      } else
        assert( isType<NILTYPE>(f) );
      assert( sl.size() == 1 );
      *rs = static_cast<resolved_state_ty *>(sl.front());
    }
    for ( smoc_port_list::iterator iter =  _ctx.ports_setup.begin();
          iter != _ctx.ports_setup.end();
          ++iter )
      (*iter)->commExec();
    TraceLog.traceEndTryExecute(actor->myModule()->name()); //
  }
  return canexec;
}

void smoc_firing_state::findBlocked(smoc_root_port_bool_list &l) {
#ifdef SYSTEMOC_DEBUG
  std::cout << "<findBlocked for "
            << fr->getActor()->myModule()->name() << ">" << std::endl;
#endif
  rs->findBlocked(l, fr->getActor());
#ifdef SYSTEMOC_DEBUG
  std::cout << "</findBlocked>" << std::endl;
#endif
}

void smoc_firing_types::resolved_state_ty::findBlocked(
    smoc_root_port_bool_list &l, smoc_root_node *actor) {
  smoc_actor *a = dynamic_cast<smoc_actor *>(actor);
  
  if ((a != NULL) && !a->vpc_event) {
    l.push_back(smoc_root_port_bool(&a->vpc_event));
//    std::cout << "XXX: " << l << std::endl;
  } else {
    for ( transitionlist_ty::iterator titer = tl.begin();
	  titer != tl.end();
	  ++titer ) {
      titer->findBlocked(l, actor);
//      std::cout << "XXX: " << l << std::endl;
    }
  }
}

void smoc_firing_types::transition_ty::findBlocked(
    smoc_root_port_bool_list &l, smoc_root_node *actor) {
  smoc_root_port_bool b      = knownSatisfiable();
  
  // std::cout << b << std::endl;
  if ( b.getStatus() != smoc_root_port_bool::IS_DISABLED )
    l.push_back(b);
}

void smoc_firing_types::transition_ty::dump(std::ostream &out) const {
  out << "transition("
        << this << ","
        << "status="   << knownSatisfiable().getStatus() << ","
//        << "knownUnsatisfiable=" << knownUnsatisfiable() << ", "
        << "ap: "                << ap << ")";
}

void smoc_firing_state::dump( std::ostream &o ) const {
  o << "state (" << this << "): ";
  for ( transitionlist_ty::const_iterator titer = rs->tl.begin();
        titer != rs->tl.end();
        ++titer )
    o << *titer << std::endl;
}






