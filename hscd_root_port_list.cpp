// vim: set sw=2 ts=8:

#include <hscd_root_port_list.hpp>

hscd_firing_state::hscd_firing_state( const hscd_transition_list &tl )
  :rs(new resolved_state_ty()), fr(NULL) {
  hscd_firing_rules *x = new hscd_firing_rules(this);
  assert( x == fr );
  for ( hscd_transition_list::const_iterator titer = tl.begin();
        titer != tl.end();
        ++titer ) {
    rs->push_back(transition_ty());
    transition_ty &t = rs->back();
    
    t.ap = titer->ap;
    t.f  = titer->ia.f;
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


