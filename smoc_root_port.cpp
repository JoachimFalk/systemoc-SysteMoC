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

#include <smoc_root_port.hpp>

#include <cosupport/oneof.hpp>

using namespace CoSupport;

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
  // std::cerr << "was here !" << std::endl;
  if ( v == IS_BLOCKED )
    reqs.push_back(e);
}

smoc_root_port_bool::smoc_root_port_bool(smoc_root_port *p, size_t n) {
  // std::cerr << "smoc_root_port_bool(smoc_root_port *p, size_t n) ";
  if ( p->availableCount() >= n ) {
    v = IS_ENABLED;// std::cerr << "enabled";
  } else if ( p->getParentPort() != NULL ||
              p->peerIsV1() ) {
    v = IS_BLOCKED;// std::cerr << "blocked";
    assert( p->getHierarchy() != _ctx.hierarchy ||
            p->peerIsV1() );
  } else {
    v = IS_DISABLED;// std::cerr << "disabled";
    assert( p->getHierarchy() == _ctx.hierarchy );
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
  // std::cerr << " "; dump(std::cerr); std::cerr << std::endl;
}

smoc_root_port_bool::smoc_root_port_bool( const this_type &a, const this_type &b )
  : v(a.v == IS_DISABLED || b.v == IS_DISABLED ? IS_DISABLED : (
      a.v == IS_ENABLED  && b.v == IS_ENABLED  ? IS_ENABLED
                                               : IS_BLOCKED ) ) {
  // std::cerr << "MERGE A:"; a.dump(std::cerr);
  // std::cerr <<      " B:"; b.dump(std::cerr);
  // std::cerr << std::endl;
  if ( v == IS_BLOCKED ) {
    if ( a.v == IS_BLOCKED )
      reqs.insert(reqs.end(), a.reqs.begin(), a.reqs.end());
    if ( b.v == IS_BLOCKED )
      reqs.insert(reqs.end(), b.reqs.begin(), b.reqs.end());
  }
  // std::cerr << "MERGE-RESULT: "; dump(std::cerr); std::cerr << std::endl;
}

smoc_root_port_bool::smoc_root_port_bool( const this_type &rhs )
  : v(rhs.v), reqs(rhs.reqs) {
  // std::cerr << "smoc_root_port_bool( const this_type &rhs ) " << rhs.reqs << std::endl;
}

smoc_root_port_bool smoc_root_port_bool::recheck() const {
  // std::cerr << "smoc_root_port_bool.recheck "; dump(std::cerr);
  // std::cerr << std::endl;
  if (v == IS_BLOCKED) {
    smoc_root_port_bool retval(true);
    
    for ( reqs_ty::const_iterator iter = reqs.begin();
          iter != reqs.end();
          ++iter ) {
      // std::cerr << "XXX: " << *iter << std::endl;
      if ( isType<smoc_commreq>(*iter) ) {
        const smoc_commreq &r = *iter;
        smoc_root_port     *p = r.first->getParentPort();
        
        assert( p != NULL );
        assert( p->getHierarchy() == r.first->getHierarchy() );
        if ( p->availableCount() < r.second ) {
          // recheck is called one hierarchy up
          if ( p->getParentPort() != NULL ) {
            assert( p->getHierarchy() != _ctx.hierarchy );
            retval.reqs.push_back( smoc_commreq(p, r.second) );
            retval.v = IS_BLOCKED; break;
          } else {
            assert( p->getHierarchy() == _ctx.hierarchy );
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

void smoc_root_port_bool::dump(std::ostream &out) const {
  out << "smoc_root_port_bool( status: "
      << v << ", ports:";
  for ( reqs_ty::const_iterator iter = reqs.begin();
        iter != reqs.end();
        ++iter )
    out << (iter != reqs.begin() ? ", " : "") << *iter;
  out << ")";
}

