// vim: set sw=2 ts=8:

#include <smoc_firing_rules.hpp>
#include <smoc_root_node.hpp>

#include <map>
#include <set>

smoc_firing_state::smoc_firing_state( const smoc_transition_list &tl )
  :rs(NULL), fr(NULL) { this->operator = (tl); }

smoc_firing_state &smoc_firing_state::operator = (const smoc_transition_list &tl) {
  if ( fr != NULL )
    fr->delRef(this);
  assert( fr == NULL );
  smoc_firing_rules *x = new smoc_firing_rules(this);
  assert( x == fr );
  rs = new resolved_state_ty();
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
  references.erase( iter ); s->fr = NULL;
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

void
smoc_firing_types::transition_ty::initTransition(
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

smoc_firing_types::maybe_transition_ty
smoc_firing_types::resolved_state_ty::findEnabledTransition() {
  for ( transitionlist_ty::iterator titer = tl.begin();
        titer != tl.end();
        ++titer ) {
    transition_ty &t = *titer;
    
    // t.reset();
    if ( !t.isBlocked() && t.knownSatisfiable() )
      return maybe_transition_ty(true,&t);
  }
  return maybe_transition_ty(false,NULL);
}

smoc_firing_types::resolved_state_ty *
smoc_firing_types::transition_ty::execute() {
  resolved_state_ty *retval = NULL;
  
  ap.commSetup();
//  assert( ap.satisfied() );
  assert( f.type() == 0 ||
          f.type() == 1 ||
	  f.type() == 2 ||
	  f.type() == 3 );
  switch (f.type()) {
    case 3: { // smoc_func_diverge
      const smoc_firing_state &ns = static_cast<smoc_func_diverge &>(f)();
      retval = &ns.getResolvedState();
      break;
    }
    case 2: { // smoc_func_branch
      const smoc_firing_state &ns = static_cast<smoc_func_branch &>(f)();
      statelist_ty::const_iterator iter = sl.begin();
      
      // check that ns is in sl
      while ( iter != sl.end() && (*iter) != ns.rs )
        ++iter;
      assert( iter != sl.end() );
      retval = &ns.getResolvedState();
      break;
    }
    case 1: // smoc_func_call
      static_cast<smoc_func_call &>(f)();
    default: { // NULL
      assert( sl.size() == 1 );
      retval = static_cast<resolved_state_ty *>(sl.front());
      break;
    }
  }
  ap.commExec();
  return retval;
}

typedef std::map<const smoc_root_port *, size_t>  pm_ty;
typedef std::set<const smoc_root_port *>          ps_ty;

//bool smoc_firing_state::inductionStep() {
//  assert( rs != NULL );
//  
//  bool again = false;
//  for ( smoc_port_list::iterator piter = getPorts().begin();
//        piter != getPorts().end();
//        ++piter ) {
//    smoc_root_port &p   = **piter;
//    size_t          min = p.maxCommittableCount();
//    size_t          max = p.committedCount();
//    
//    for ( transitionlist_ty::iterator titer = rs->tl.begin();
//          titer != rs->tl.end();
//          ++titer ) {
//      smoc_activation_pattern &ap = titer->ap;
//      
//      if ( !ap.knownUnsatisfiable() ) {
//        smoc_activation_pattern::iterator opiter = ap.find(&p);
//        if ( opiter != ap.end() ) {
//          if ( opiter->second.commitCount() < min )
//            min = opiter->second.commitCount();
//          if ( opiter->second.commitCount() > max )
//            max = opiter->second.commitCount();
//        } else {
//          min = 0;
//        }
//      }
//    }
//    again |= p.setCommittedCount( min > max ? max : min );
//    again |= p.setMaxCommittable( max );
//  }
//  return again;
//}

bool smoc_firing_state::inductionStep() {
  assert( rs != NULL );
  
  bool again = false;
/*
  for ( smoc_port_list::iterator piter = getPorts().begin();
        piter != getPorts().end();
        ++piter ) {
    smoc_root_port &p   = **piter;
    size_t          min = ~0; // p.maxCommittableCount();
    size_t          max = p.committedCount();
    
    for ( transitionlist_ty::iterator titer = rs->tl.begin();
          titer != rs->tl.end();
          ++titer ) {
      smoc_activation_pattern &ap = titer->ap;
      
      if ( !titer->knownUnsatisfiable() ) {
        smoc_activation_pattern::iterator opiter = ap.find(&p);
        if ( opiter != ap.end() ) {
          if ( opiter->second.commitCount() < min )
            min = opiter->second.commitCount();
          if ( opiter->second.commitCount() > max )
            max = opiter->second.commitCount();
        } else {
          min = 0;
        }
      }
    }
    again |= p.setCommittedCount( min > max ? max : min );
//    again |= p.setMaxCommittable( max );
  }
 */
  return again;
}

bool smoc_firing_state::choiceStep() {
  assert( rs != NULL );
  
/*
  for ( transitionlist_ty::iterator titer = rs->tl.begin();
        titer != rs->tl.end();
        ++titer ) {
    if ( titer->knownSatisfiable() ) {
      bool again = false;
      for ( smoc_port_list::iterator piter = getPorts().begin();
            piter != getPorts().end();
            ++piter ) {
        smoc_root_port &p   = **piter;
        smoc_activation_pattern::iterator opiter = ap.find(&p);
        size_t commitCount = opiter != ap.end()
          ? opiter->second.commitCount() : 0;
        again |= p.setCommittedCount(commitCount);
//        again |= p.setMaxCommittable(commitCount);
      }
      return again;
    }
  }
*/
  return false;
}

void smoc_firing_types::transition_ty::dump(std::ostream &out) const {
  out << "transition("
        << this << ","
        << "knownSatisfiable="   << knownSatisfiable() << ","
        << "knownUnsatisfiable=" << knownUnsatisfiable() << ", "
        << "ap: "                << ap << ")";
}

void smoc_firing_state::dump( std::ostream &o ) const {
  o << "state (" << this << "): ";
  for ( transitionlist_ty::const_iterator titer = rs->tl.begin();
        titer != rs->tl.end();
        ++titer )
    o << *titer << std::endl;
}
