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
    rs->tl.push_back(transition_ty());
    transition_ty &t = rs->tl.back();
    
    t.ap = titer->ap;
    t.f  = titer->ia.f;
//    for ( hscd_activation_pattern::iterator iter = t.ap.begin();
//          iter != t.ap.end();
//          ++iter )
//      rs->ps.insert(iter->second.getPort());
    for ( hscd_firing_state_list::const_iterator siter = titer->ia.sl.begin();
          siter != titer->ia.sl.end();
          ++siter ) {
      // typeof(*siter) == oneof<hscd_firing_state *, hscd_firing_state>
      assert( siter->type() == 1 || siter->type() == 2 );
      if ( siter->type() == 1 ) {
        // unresolved state
        t.sl.push_back(static_cast<hscd_firing_state *>(*siter));
        fr->addUnresolved(&t.sl.back());
      } else {
        // resolved state
        t.sl.push_back(static_cast<hscd_firing_state>(*siter).rs);
        fr->unify(static_cast<hscd_firing_state>(*siter).fr);
      }
    }
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
  assert( fr != NULL && fr->actor == NULL );
  fr->actor = actor;
}

hscd_port_list &hscd_firing_state::getPorts() const {
  assert( fr != NULL && fr->actor != NULL );
  return fr->actor->getPorts();
}

typedef std::map<const hscd_root_port *, size_t>  pm_ty;
typedef std::set<const hscd_root_port *>          ps_ty;

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
      
      if ( ap.satisfiable() ) {
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
    
    if ( ap.canSatisfy() ) {
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
