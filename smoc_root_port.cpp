// vim: set sw=2 ts=8:

#include <smoc_root_port.hpp>

#include <cosupport/oneof.hpp>

using namespace jflibs;

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
    _ctx.blocked &= *e;
}

smoc_root_port_bool::smoc_root_port_bool(smoc_root_port *p, size_t n) {
  // std::cout << "smoc_root_port_bool(smoc_root_port *p, size_t n) ";
  if ( p->availableCount() >= n ) {
    v = IS_ENABLED;// std::cout << "enabled";
  } else if ( /* p->getParentPort() != NULL || */
              p->peerIsV1() ) {
    v = IS_BLOCKED;// std::cout << "blocked";
    // assert( p->getHierarchy() != _ctx.hierarchy ||
    //         p->peerIsV1() );
  } else {
    v = IS_DISABLED;// std::cout << "disabled";
    // assert( p->getHierarchy() == _ctx.hierarchy );
  }
  switch ( v ) {
    case IS_ENABLED:
      p->commSetup(n);
      // _ctx.ports_setup.push_back(p);
      break;
    case IS_BLOCKED:
      p->blockEvent().reset();
      _ctx.blocked &= p->blockEvent();
      break;
    default:
      break;
  }
  // std::cout << " "; dump(std::cout); std::cout << std::endl;
}

smoc_root_port_bool::smoc_root_port_bool( const this_type &a, const this_type &b )
  : v(a.v == IS_DISABLED || b.v == IS_DISABLED ? IS_DISABLED : (
      a.v == IS_ENABLED  && b.v == IS_ENABLED  ? IS_ENABLED
                                               : IS_BLOCKED ) ) {}

smoc_root_port_bool::smoc_root_port_bool( const this_type &rhs )
  : v(rhs.v) {}

/*
smoc_root_port_bool smoc_root_port_bool::recheck() const {
  // std::cout << "smoc_root_port_bool.recheck "; dump(std::cout);
  // std::cout << std::endl;
  if (v == IS_BLOCKED) {
    smoc_root_port_bool retval(true);
    
    for ( reqs_ty::const_iterator iter = reqs.begin();
          iter != reqs.end();
          ++iter ) {
      // std::cout << "XXX: " << *iter << std::endl;
      if ( isType<smoc_commreq>(*iter) ) {
        const smoc_commreq &r = *iter;
        smoc_root_port     *p = r.first->getParentPort();
        
        assert( p != NULL );
        assert( p->getHierarchy() == r.first->getHierarchy() );
        if ( p->availableCount() < r.second ) {
          // recheck is called one hierarchy up
          if ( p->getParentPort() != NULL ) {
            // assert( p->getHierarchy() != _ctx.hierarchy );
            retval.reqs.push_back( smoc_commreq(p, r.second) );
            retval.v = IS_BLOCKED; break;
          } else {
            // assert( p->getHierarchy() == _ctx.hierarchy );
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
    return retval;
  } else
    return *this;
}
*/

void smoc_root_port_bool::dump(std::ostream &out) const {
  out << "smoc_root_port_bool( status: "
      << v << ")";
}
