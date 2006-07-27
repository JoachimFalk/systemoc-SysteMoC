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

#ifndef _INCLUDED_SMOC_ROOT_PORT_HPP
#define _INCLUDED_SMOC_ROOT_PORT_HPP

#include <cosupport/commondefs.h>

#include <iostream>
#include <cassert>

#include <list>
#include <utility>

#include <cosupport/stl_output_for_list.hpp>
#include <cosupport/stl_output_for_pair.hpp>
#include <cosupport/oneof.hpp>

#include <systemc.h>

#include <smoc_expr.hpp>
#include <smoc_event.hpp>

class smoc_root_port
  // must be public inheritance for dynamic_cast in smoc_root_node to work
  : public sc_port_base {
public:
  typedef smoc_root_port  this_type;
  
  template <class E> friend class Expr::Value;
//  friend class smoc_firing_types::resolved_state_ty;
//  friend class smoc_firing_types::transition_ty;
protected:
  smoc_root_port *parent;
  
  smoc_root_port( const char* name_ )
    : sc_port_base( name_, 1 ), parent(NULL), is_smoc_v1_port(false) {}
public:
  virtual void commSetup(size_t req)                = 0;
#ifdef ENABLE_SYSTEMC_VPC
  virtual void commExec(const smoc_ref_event_p &le) = 0;
#else
  virtual void commExec()                           = 0;
#endif
  virtual void reset()                              = 0;
public:
  bool is_smoc_v1_port;
  
  static const char* const kind_string;
  virtual const char* kind() const
    { return kind_string; }
  
  virtual size_t      availableCount() const = 0;
  virtual smoc_event &blockEvent(size_t n = MAX_TYPE(size_t)) = 0;
  virtual bool        isInput() const = 0;
  bool                isOutput() const
    { return !isInput(); }
  
  virtual bool peerIsV1() const = 0;
  
  virtual void clearReady()
    { assert( !"SHOULD NEVER BE CALLED !!!" ); }
  virtual void communicate( size_t n )
    { assert( !"SHOULD NEVER BE CALLED !!!" ); }
  
  smoc_root_port *getParentPort() const
    { return parent; }
  
  // bind interface to this port
  void bind( sc_interface& interface_ ) { sc_port_base::bind(interface_); }
  // bind parent port to this port
  void bind( this_type &parent_ ) {
    assert( parent == NULL ); parent = &parent_;
    sc_port_base::bind(parent_);
  }
  
  void dump( std::ostream &out ) const;
private:
  // disabled
  smoc_root_port( const this_type & );
  this_type& operator = ( const this_type & );
};

static inline
std::ostream &operator <<( std::ostream &out, const smoc_root_port &p )
  { p.dump(out); return out; }

typedef std::list<smoc_root_port *> smoc_port_list;

class smoc_root_port_in
: public smoc_root_port {
public:
  smoc_root_port_in( const char* name_ )
    : smoc_root_port(name_) {}
};

class smoc_root_port_out
: public smoc_root_port {
public:
  smoc_root_port_out( const char* name_ )
    : smoc_root_port(name_) {}
};

#endif // _INCLUDED_SMOC_ROOT_PORT_HPP
