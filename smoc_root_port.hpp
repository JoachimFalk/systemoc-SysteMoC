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
  virtual void commSetup(size_t req) = 0;
  virtual void commExec()            = 0;
  virtual void reset()               = 0;
public:
  bool is_smoc_v1_port;
  
  static const char* const kind_string;
  virtual const char* kind() const
    { return kind_string; }
  
  virtual sc_module  *getHierarchy() const = 0;
  virtual size_t      availableCount() const = 0;
  virtual smoc_event &blockEvent() = 0;
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

struct smoc_ctx {
  sc_module       *hierarchy;
  smoc_port_list   ports_setup;
  
  smoc_ctx()
    : hierarchy(NULL) {}
};

extern smoc_ctx _ctx;



static inline
class smoc_root_port_bool operator >= (class smoc_commnr c, size_t n);

class smoc_commnr {
public:
  typedef smoc_commnr this_type;
  
  friend smoc_root_port_bool operator >= (smoc_commnr c, size_t n);
  
  void dump( std::ostream &out ) const 
    { out << "commnr(" << &p << ")"; }
private:
  smoc_root_port &p;
public:
  
  smoc_commnr(smoc_root_port &p) : p(p) {}
};

static inline
std::ostream &operator <<( std::ostream &out, const smoc_commnr &p )
  { p.dump(out); return out; }

typedef std::pair<smoc_root_port *, size_t>  smoc_commreq;

class smoc_root_port_bool {
public:
  typedef smoc_root_port_bool this_type;
  typedef void (this_type::*unspecified_bool_type)();
  
  enum status_ty {
    IS_DISABLED,  // this guard is definitely false
    IS_BLOCKED,   // this guard is blocked
    IS_ENABLED    // this guard is still true
  };
  
  template <typename T_top>
  friend class smoc_top_moc;
private:
  typedef std::list<CoSupport::oneof<smoc_commreq,smoc_event *> > reqs_ty;
  
//  void dummy() {};
//  operator unspecified_bool_type() const
//    { return v ? &this_type::dummy : NULL; }
  
  status_ty v;
  reqs_ty   reqs;
protected:
public:
  smoc_root_port_bool( bool v = false );
  smoc_root_port_bool( smoc_event *e );
  smoc_root_port_bool( smoc_root_port *p, size_t n );
  smoc_root_port_bool( const this_type &a, const this_type &b );
  smoc_root_port_bool( const this_type &rhs );
  
  smoc_root_port_bool recheck() const;
  
  status_ty getStatus() const { return v; }
  
  void dump(std::ostream &out) const;
};

typedef std::list<smoc_root_port_bool> smoc_root_port_bool_list;

static inline
std::ostream &operator <<( std::ostream &out, const smoc_root_port_bool &p )
  { p.dump(out); return out; }

static inline
smoc_root_port_bool operator >= (smoc_commnr c, size_t n)
  { return smoc_root_port_bool(&c.p,n); }

namespace Expr {

/****************************************************************************
 * DGuard represents a virtual guard which hides an smoc_root_port_bool object
 */

struct ASTNodeVGuard: public ASTNodeTerminal {
};

class DVGuard {
public:
  typedef smoc_root_port_bool value_type;
  typedef DVGuard              this_type;
  
  friend class Value<this_type>;
  friend class AST<this_type>;
private:
  const value_type v;
public:
  explicit DVGuard(const value_type &v): v(v) {}
};

template <>
struct Value<DVGuard> {
  typedef DVGuard::value_type result_type;
  
  static inline
  result_type apply(const DVGuard &e)
    { return e.v.recheck(); }
};

template <>
struct AST<DVGuard> {
  typedef PASTNode result_type;
  
  static inline
  PASTNode apply(const DVGuard &e)
    { return PASTNode(new ASTNodeVGuard()); }
};

template <>
struct D<DVGuard>: public DBase<DVGuard> {
  D(const smoc_root_port_bool &v): DBase<DVGuard>(DVGuard(v)) {}
};

// Make a convenient typedef for the placeholder type.
struct VGuard { typedef D<DVGuard> type; };

static inline
VGuard::type vguard(const smoc_root_port_bool &v)
  { return VGuard::type(v); }

/****************************************************************************
 * DSMOCEvent represents a smoc_event guard which turns true if the event is
 * signaled
 */

struct ASTNodeSMOCEvent: public ASTNodeTerminal {
};

class DSMOCEvent {
public:
  typedef smoc_event value_type;
  typedef DSMOCEvent this_type;
  
  friend class Value<this_type>;
  friend class AST<this_type>;
private:
  value_type &v;
public:
  explicit DSMOCEvent(value_type &v): v(v) {}
};

template <>
struct Value<DSMOCEvent> {
  typedef smoc_root_port_bool result_type;
  
  static inline
  result_type apply(const DSMOCEvent &e)
    { return smoc_root_port_bool(&e.v); }
};

template <>
struct AST<DSMOCEvent> {
  typedef PASTNode result_type;
  
  static inline
  PASTNode apply(const DSMOCEvent &e)
    { return PASTNode(new ASTNodeSMOCEvent()); }
};

template <>
struct D<DSMOCEvent>: public DBase<DSMOCEvent> {
  D(smoc_event &v): DBase<DSMOCEvent>(DSMOCEvent(v)) {}
};

// Make a convenient typedef for the placeholder type.
struct SMOCEvent { typedef D<DSMOCEvent> type; };

static inline
SMOCEvent::type till(smoc_event &e)
  { return SMOCEvent::type(e); }

/****************************************************************************/

// NEEDED:
//  to implement short circuit boolean evaluation
//  with smoc_root_port_bool
template <typename T>
struct DBinOpExecute<smoc_root_port_bool,T,DOpBinLAnd> {
  typedef smoc_root_port_bool result_type;
  
  template <class A, class B>
  static inline
  result_type apply( const A &a, const B &b ) {
//    std::cerr << "foo" << std::endl;
    result_type ra =  Value<A>::apply(a);
    return ra.getStatus() == smoc_root_port_bool::IS_ENABLED
      ? ( Value<B>::apply(b)
          ? ra
          : result_type() )
      : ra;
  }
};

// NEEDED:
//  to implement short circuit boolean evaluation
//  with smoc_root_port_bool
template <typename T>
struct DBinOpExecute<T,smoc_root_port_bool,DOpBinLAnd> {
  typedef smoc_root_port_bool result_type;
  
  template <class A, class B>
  static inline
  result_type apply( const A &a, const B &b ) {
//    std::cerr << "hix" << std::endl;
    return Value<A>::apply(a)
      ? Value<B>::apply(b)
      : result_type();
  }
};

// NEEDED:
//  to implement short circuit boolean evaluation
//  with smoc_root_port_bool
template <>
struct DBinOpExecute<smoc_root_port_bool,smoc_root_port_bool,DOpBinLAnd> {
  typedef smoc_root_port_bool result_type;
  
  template <class A, class B>
  static inline
  result_type apply( const A &a, const B &b ) {
//    std::cerr << "bar" << std::endl;
    
    result_type ra(Value<A>::apply(a));
    if ( ra.getStatus() == smoc_root_port_bool::IS_ENABLED )
      return result_type(ra, Value<B>::apply(b));
    else
      return ra;
  }
};

}

#endif // _INCLUDED_SMOC_ROOT_PORT_HPP
