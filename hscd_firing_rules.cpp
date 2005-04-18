// vim: set sw=2 ts=8:

#include <hscd_firing_rules.hpp>
#include <hscd_root_node.hpp>

#include <map>
#include <set>

hscd_firing_state::hscd_firing_state( const hscd_transition_list &tl )
  :rs(new resolved_state_ty()), fr(NULL) {
  hscd_firing_rules *x = new hscd_firing_rules(this);
  assert( x == fr );
  for ( hscd_transition_list::const_iterator titer = tl.begin();
        titer != tl.end();
        ++titer ) {
    transition_ty &t = rs->addTransition();
    t.initTransition(fr,*titer);
  }
  fr->resolve();
}

hscd_firing_state &hscd_firing_state::operator = (const this_type &x) {
  if ( &x != this ) {
    if ( fr )
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

void hscd_firing_rules::_addRef( hscd_firing_state *s, hscd_firing_rules *p ) {
  assert( s != NULL );
  bool success = references.insert(s).second;
  assert( success && s->fr == p );
  s->fr = this;
}

void hscd_firing_rules::delRef( hscd_firing_state *s ) {
  references_ty::iterator iter = references.find(s);
  
  assert( iter != references.end() && s->fr == this );
  references.erase( iter ); s->fr = NULL;
  if ( references.empty() )
    delete this;
}

void hscd_firing_rules::resolve() {
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

void hscd_firing_rules::unify( hscd_firing_rules *fr ) {
  hscd_firing_rules *src, *dest;
  
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

void hscd_firing_state::finalise( hscd_root_node *actor ) const {
  if ( fr == NULL )
    return;
  assert( fr != NULL );
  fr->finalise(actor);
}

void hscd_firing_rules::finalise( hscd_root_node *actor_ ) {
  assert( unresolved_states.empty() );
  assert( actor == NULL );
  actor = actor_;
}


hscd_port_list &hscd_firing_state::getPorts() const {
  assert( fr != NULL && fr->actor != NULL );
  return fr->actor->getPorts();
}

void
hscd_firing_types::transition_ty::initTransition(
    hscd_firing_rules *fr,
    const hscd_interface_transition &t ) {
  ap = t.ap; f = t.ia.f;
  for ( hscd_firing_state_list::const_iterator siter = t.ia.sl.begin();
        siter != t.ia.sl.end();
        ++siter ) {
    // typeof(*siter) == oneof<hscd_firing_state *, hscd_firing_state>
    assert( siter->type() == 1 || siter->type() == 2 );
    if ( siter->type() == 1 ) {
      // unresolved state
      sl.push_back(static_cast<hscd_firing_state *>(*siter));
      fr->addUnresolved(&sl.back());
    } else {
      // resolved state
      sl.push_back(static_cast<hscd_firing_state>(*siter).rs);
      fr->unify(static_cast<hscd_firing_state>(*siter).fr);
    }
  }
}


hscd_firing_types::maybe_transition_ty
hscd_firing_types::resolved_state_ty::findEnabledTransition() {
  if (_blocked == NULL) {
    for ( transitionlist_ty::iterator titer = tl.begin();
          titer != tl.end();
          ++titer ) {
      transition_ty &t = *titer;
      hscd_activation_pattern &ap = t.ap;
      
      ap.reset();
      if ( ap.knownSatisfiable() )
        return maybe_transition_ty(true,&t);
    }
  }
  return maybe_transition_ty(false,NULL);
}

hscd_firing_types::resolved_state_ty *
hscd_firing_types::transition_ty::execute() {
  ap.execute();
  assert( ap.satisfied() );
  assert( f.type() == 0 || f.type() == 1 || f.type() == 2 );
  switch (f.type()) {
    case 2: { // hscd_func_branch
      const hscd_firing_state &ns = static_cast<hscd_func_branch>(f)();
      statelist_ty::const_iterator iter = sl.begin();
      
      while ( iter != sl.end() && (*iter) != ns.rs )
        ++iter;
      assert( iter != sl.end() );
      return &ns.getResolvedState();
      break;
    }
    case 1: // hscd_func_call
      static_cast<hscd_func_call>(f)();
    default: { // NULL
      assert( sl.size() == 1 );
      return static_cast<resolved_state_ty *>(sl.front());
      break;
    }
  }
}

typedef std::map<const hscd_root_port *, size_t>  pm_ty;
typedef std::set<const hscd_root_port *>          ps_ty;

//bool hscd_firing_state::inductionStep() {
//  assert( rs != NULL );
//  
//  bool again = false;
//  for ( hscd_port_list::iterator piter = getPorts().begin();
//        piter != getPorts().end();
//        ++piter ) {
//    hscd_root_port &p   = **piter;
//    size_t          min = p.maxCommittableCount();
//    size_t          max = p.committedCount();
//    
//    for ( transitionlist_ty::iterator titer = rs->tl.begin();
//          titer != rs->tl.end();
//          ++titer ) {
//      hscd_activation_pattern &ap = titer->ap;
//      
//      if ( !ap.knownUnsatisfiable() ) {
//        hscd_activation_pattern::iterator opiter = ap.find(&p);
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

bool hscd_firing_state::inductionStep() {
  assert( rs != NULL );
  
  bool again = false;
  for ( hscd_port_list::iterator piter = getPorts().begin();
        piter != getPorts().end();
        ++piter ) {
    hscd_root_port &p   = **piter;
    size_t          min = p.maxCommittableCount();
    size_t          max = p.committedCount();
    
    for ( transitionlist_ty::iterator titer = rs->tl.begin();
          titer != rs->tl.end();
          ++titer ) {
      hscd_activation_pattern &ap = titer->ap;
      
      if ( !ap.knownUnsatisfiable() ) {
        hscd_activation_pattern::iterator opiter = ap.find(&p);
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
    again |= p.setMaxCommittable( max );
  }
  return again;
}

bool hscd_firing_state::choiceStep() {
  assert( rs != NULL );
  
  for ( transitionlist_ty::iterator titer = rs->tl.begin();
        titer != rs->tl.end();
        ++titer ) {
    hscd_activation_pattern &ap = titer->ap;
    
    if ( ap.knownSatisfiable() ) {
      bool again = false;
      for ( hscd_port_list::iterator piter = getPorts().begin();
            piter != getPorts().end();
            ++piter ) {
        hscd_root_port &p   = **piter;
        hscd_activation_pattern::iterator opiter = ap.find(&p);
        size_t commitCount = opiter != ap.end()
          ? opiter->second.commitCount() : 0;
        again |= p.setCommittedCount(commitCount);
        again |= p.setMaxCommittable(commitCount);
      }
      return again;
    }
  }
  return false;
}

void hscd_firing_state::dump( std::ostream &o ) const {
  for ( transitionlist_ty::const_iterator titer = rs->tl.begin();
        titer != rs->tl.end();
        ++titer ) {
    const transition_ty &t = *titer;
    const hscd_activation_pattern &ap = t.ap;
    
    o << "  transition(" << &t << ","
                       "knownSatisfiable=" << ap.knownSatisfiable() << ","
                       "knownUnsatisfiable=" << ap.knownUnsatisfiable() << ")" << std::endl;
    for ( hscd_activation_pattern::const_iterator apiter = ap.begin();
          apiter != ap.end();
          ++apiter ) {
      const hscd_op_port   &op = apiter->second;
      const hscd_root_port *p  = op.getPort();
      o << "    " << *p << std::endl;
    }
  }
}
