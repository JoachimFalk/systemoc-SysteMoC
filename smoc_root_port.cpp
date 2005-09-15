// vim: set sw=2 ts=8:

#include <smoc_root_port.hpp>

smoc_ctx _ctx;

void smoc_root_port::dump( std::ostream &out ) const {
  out << "port(" << this
      <<      ",name=" << name()
      <<      ",hierarchy=" << getHierarchy()->name()
      <<      ",available=" << availableCount() << ")";
}

const char* const smoc_root_port::kind_string = "smoc_root_port";

smoc_root_port_bool::smoc_root_port_bool( bool v )
  : v(v ? IS_ENABLED : IS_DISABLED) {}

smoc_root_port_bool::smoc_root_port_bool( smoc_event *e )
  : v( *e ? IS_ENABLED : IS_BLOCKED ) {
  // std::cout << "was here !" << std::endl;
  if ( v == IS_BLOCKED )
    reqs.push_back(e);
}

smoc_root_port_bool::smoc_root_port_bool(smoc_root_port *p, size_t n) {
  // std::cout << "smoc_root_port_bool(smoc_root_port *p, size_t n) ";
  if ( p->availableCount() >= n ) {
    v = IS_ENABLED;// std::cout << "enabled";
  } else if ( p->getHierarchy() != _ctx.hierarchy ||
              p->peerIsV1() ) {
    v = IS_BLOCKED;// std::cout << "blocked";
    assert( p->getParentPort() != NULL || p->peerIsV1() );
  } else {
    v = IS_DISABLED;// std::cout << "disabled";
    assert( p->getParentPort() == NULL );
  }
  if ( v == IS_ENABLED ) {
    p->commSetup(n);
    _ctx.ports_setup.push_back(p);
  }
  if ( v == IS_BLOCKED ) {
    if ( p->peerIsV1() ) {
      p->blockEvent().reset();
      reqs.push_back(&p->blockEvent());
    } else {
      reqs.push_back(smoc_commreq(p,n));
    }
  }
  // std::cout << " "; dump(std::cout); std::cout << std::endl;
}

smoc_root_port_bool::smoc_root_port_bool( const this_type &a, const this_type &b )
  : v(a.v == IS_DISABLED || b.v == IS_DISABLED ? IS_DISABLED : (
      a.v == IS_ENABLED  && b.v == IS_ENABLED  ? IS_ENABLED
                                               : IS_BLOCKED ) ) {
  // std::cout << "MERGE A:"; a.dump(std::cout);
  // std::cout <<      " B:"; b.dump(std::cout);
  // std::cout << std::endl;
  if ( v == IS_BLOCKED ) {
    if ( a.v == IS_BLOCKED )
      reqs.insert(reqs.end(), a.reqs.begin(), a.reqs.end());
    if ( b.v == IS_BLOCKED )
      reqs.insert(reqs.end(), b.reqs.begin(), b.reqs.end());
  }
  // std::cout << "MERGE-RESULT: "; dump(std::cout); std::cout << std::endl;
}

smoc_root_port_bool::smoc_root_port_bool( const this_type &rhs )
  : v(rhs.v), reqs(rhs.reqs) {
  // std::cout << "smoc_root_port_bool( const this_type &rhs ) " << rhs.reqs << std::endl;
}

smoc_root_port_bool smoc_root_port_bool::recheck() const {
  smoc_root_port_bool retval;
  
  // std::cout << "smoc_root_port_bool.recheck "; dump(std::cout);
  // std::cout << std::endl;
  if (v == IS_BLOCKED) {
    retval.v = IS_ENABLED;
    for ( reqs_ty::const_iterator iter = reqs.begin();
          iter != reqs.end();
          ++iter ) {
      // std::cout << "XXX: " << *iter << std::endl;
      if ( isType<smoc_commreq>(*iter) ) {
        const smoc_commreq &r = *iter;
        
        if ( r.first->availableCount() < r.second ) {
          if ( r.first->getHierarchy() != _ctx.hierarchy ) {
            retval.reqs.push_back(r);
            retval.v = IS_BLOCKED; break;
          } else {
            retval.reqs.clear();
            retval.v = IS_DISABLED; break;
          }
        }
      } else {
        smoc_event *e = *iter;
        if ( !*e ) {
          retval.reqs.push_back(e);
          retval.v = IS_BLOCKED; break;
        }
      }
    }
  }
  // retval.dump(std::cout);
  return retval;
}

void smoc_root_port_bool::dump(std::ostream &out) const {
  out << "smoc_root_port_bool( status: "
      << v << ", ports:";
  for ( reqs_ty::const_iterator iter = reqs.begin();
        iter != reqs.end();
        ++iter )
    out << (iter != reqs.begin() ? ", " : "") << *iter;
  out << ")";
}

