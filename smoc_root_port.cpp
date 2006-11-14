// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2006 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 * 
 * --- This software and any associated documentation is provided "as is" 
 * 
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 */

#include <smoc_root_port.hpp>

#include <cosupport/oneof.hpp>

using namespace CoSupport;

// smoc_ctx _ctx;

void smoc_root_port::dump( std::ostream &out ) const {
  out << "port(" << this
      <<      ",name=" << name()
      <<      ",hierarchy=" << getHierarchy()->name()
      <<      ",available=" << availableCount() << ")";
}

const char* const smoc_root_port::kind_string = "smoc_root_port";

/*

smoc_root_port_bool::smoc_root_port_bool( bool v )
  : v(v ? IS_ENABLED : IS_DISABLED) {}

smoc_root_port_bool::smoc_root_port_bool( smoc_event *e )
  : v( *e ? IS_ENABLED : IS_BLOCKED ) {
  // std::cerr << "was here !" << std::endl;
  if ( _ctx.blocked && v == IS_BLOCKED )
    *_ctx.blocked &= *e;
}

smoc_root_port_bool::smoc_root_port_bool(smoc_root_port *p, size_t n) {
#ifdef SYSTEMOC_DEBUG
  std::cerr << "smoc_root_port_bool(smoc_root_port *p == " << *p << ", size_t n == " << n << ") ";
#endif
  if ( p->availableCount() >= n ) {
    v = IS_ENABLED;
    p->commSetup(n);
#ifdef SYSTEMOC_DEBUG
    std::cerr << "enabled" << std::endl;
#endif
  } else if ( // p->getParentPort() != NULL ||
              p->peerIsV1() ) {
    v = IS_BLOCKED;
    p->blockEvent().reset();
    if ( _ctx.blocked )
      *_ctx.blocked &= p->blockEvent();
#ifdef SYSTEMOC_DEBUG
    std::cerr << "blocked" << std::endl;
#endif
    // assert( p->getHierarchy() != _ctx.hierarchy ||
    //         p->peerIsV1() );
  } else {
    v = IS_DISABLED;
#ifdef SYSTEMOC_DEBUG
    std::cerr << "disabled" << std::endl;
#endif
    // assert( p->getHierarchy() == _ctx.hierarchy );
  }
}

smoc_root_port_bool::smoc_root_port_bool( const this_type &a, const this_type &b )
  : v(a.v == IS_DISABLED || b.v == IS_DISABLED ? IS_DISABLED : (
      a.v == IS_ENABLED  && b.v == IS_ENABLED  ? IS_ENABLED
                                               : IS_BLOCKED ) ) {}

smoc_root_port_bool::smoc_root_port_bool( const this_type &rhs )
  : v(rhs.v) {}

void smoc_root_port_bool::dump(std::ostream &out) const {
  out << "smoc_root_port_bool( status: "
      << v << ")";
}

*/
