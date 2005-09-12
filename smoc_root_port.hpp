// vim: set sw=2 ts=8:

#ifndef _INCLUDED_SMOC_ROOT_PORT_HPP
#define _INCLUDED_SMOC_ROOT_PORT_HPP

#include <iostream>
#include <cassert>

#include <list>
#include <hscdsupport/stl_output_for_list.hpp>

#include <utility>
#include <hscdsupport/stl_output_for_pair.hpp>

#include <hscdsupport/oneof.hpp>

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
  smoc_root_port( const char* name_ )
    : sc_port_base( name_, 1 ), is_smoc_v1_port(false) {}
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
  
/*
  // bind interface to this port
  void bind( sc_interface& interface_ ) { sc_port_base::bind(interface_); }
  // bind parent port to this port
  void bind( this_type &parent_ ) { uplevel = true; sc_port_base::bind(parent_); }
*/
  
  void dump( std::ostream &out ) const {
    out << "port(" << this
        <<      ",name=" << name()
        <<      ",hierarchy=" << getHierarchy()->name()
        <<      ",available=" << availableCount() << ")";
  }
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
  template <typename T> friend class smoc_port_in;
  template <typename T> friend class smoc_port_out;
  
  void dump( std::ostream &out ) const 
    { out << "commnr(" << p << ")"; }
private:
  smoc_root_port &p;
  
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
  typedef std::list<oneof<smoc_commreq,smoc_event *> > reqs_ty;
  
//  void dummy() {};
//  operator unspecified_bool_type() const
//    { return v ? &this_type::dummy : NULL; }
  
  status_ty v;
  reqs_ty   reqs;
protected:
public:
  smoc_root_port_bool( bool v = false )
    : v(v ? IS_ENABLED : IS_DISABLED) {}
  smoc_root_port_bool(smoc_root_port *p, size_t n) {
//    std::cout << "smoc_root_port_bool(smoc_root_port *p, size_t n) ";
    if ( p->availableCount() >= n ) {
      v = IS_ENABLED;// std::cout << "enabled";
    } else if ( p->getHierarchy() != _ctx.hierarchy ||
                p->peerIsV1() ) {
      v = IS_BLOCKED;// std::cout << "blocked";
    } else {
      v = IS_DISABLED;// std::cout << "disabled";
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
//    std::cout << " "; dump(std::cout); std::cout << std::endl;
  }
  smoc_root_port_bool( smoc_event *e )
    : v( *e ? IS_ENABLED : IS_BLOCKED ) {
//    std::cout << "was here !" << std::endl;
    if ( v == IS_BLOCKED )
      reqs.push_back(e);
  }
  smoc_root_port_bool( const this_type &a, const this_type &b )
    : v(a.v == IS_DISABLED || b.v == IS_DISABLED ? IS_DISABLED : (
        a.v == IS_ENABLED  && b.v == IS_ENABLED  ? IS_ENABLED
                                                 : IS_BLOCKED ) ) {
//    std::cout << "MERGE A:"; a.dump(std::cout);
//    std::cout <<      " B:"; b.dump(std::cout);
//    std::cout << std::endl;
    if ( v == IS_BLOCKED ) {
      if ( a.v == IS_BLOCKED )
        reqs.insert(reqs.end(), a.reqs.begin(), a.reqs.end());
      if ( b.v == IS_BLOCKED )
        reqs.insert(reqs.end(), b.reqs.begin(), b.reqs.end());
    }
//    std::cout << "MERGE-RESULT: "; dump(std::cout); std::cout << std::endl;
  }
  smoc_root_port_bool( const this_type &rhs )
    : v(rhs.v), reqs(rhs.reqs) {
//    std::cout << "smoc_root_port_bool( const this_type &rhs ) " << rhs.reqs << std::endl;
  }
  
  smoc_root_port_bool recheck() const {
    smoc_root_port_bool retval;
    
//    std::cout << "smoc_root_port_bool.recheck "; dump(std::cout);
//    std::cout << std::endl;
    if (v == IS_BLOCKED) {
      retval.v = IS_ENABLED;
      for ( reqs_ty::const_iterator iter = reqs.begin();
            iter != reqs.end();
            ++iter ) {
//        std::cout << "XXX: " << *iter << std::endl;
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
//    retval.dump(std::cout);
    return retval;
  }
  
  status_ty getStatus() const { return v; }
  
  void dump(std::ostream &out) const {
    out << "smoc_root_port_bool( status: "
        << v << ", ports:";
    for ( reqs_ty::const_iterator iter = reqs.begin();
          iter != reqs.end();
          ++iter )
      out << (iter != reqs.begin() ? ", " : "") << *iter;
    out << ")";
  }
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

struct Value<DVGuard> {
  typedef DVGuard::value_type result_type;
  
  static inline
  result_type apply(const DVGuard &e)
    { return e.v.recheck(); }
};

struct AST<DVGuard> {
  typedef PASTNode result_type;
  
  static inline
  PASTNode apply(const DVGuard &e)
    { return PASTNode(new ASTNodeVGuard()); }
};

struct D<DVGuard>: public DBase<DVGuard> {
  D(const smoc_root_port_bool &v): DBase<DVGuard>(DVGuard(v)) {}
};

// Make a convenient typedef for the placeholder type.
struct VGuard { typedef D<DVGuard> type; };

static inline
VGuard::type vguard(const smoc_root_port_bool &v)
  { return VGuard::type(v); }

static inline
VGuard::type till(smoc_event &e)
  { return VGuard::type(&e); }

// NEEDED:
//  to implement short circuit boolean evaluation
//  with smoc_root_port_bool
template <typename T>
struct DBinOpExecute<smoc_root_port_bool,T,DOpBinLAnd> {
  typedef smoc_root_port_bool result_type;
  
  template <class A, class B>
  static inline
  result_type apply( const A &a, const B &b ) {
//    std::cout << "foo" << std::endl;
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
//    std::cout << "hix" << std::endl;
    return Value<A>::apply(a)
      ? Value<B>::apply(b)
      : result_type();
  }
};

// NEEDED:
//  to implement short circuit boolean evaluation
//  with smoc_root_port_bool
struct DBinOpExecute<smoc_root_port_bool,smoc_root_port_bool,DOpBinLAnd> {
  typedef smoc_root_port_bool result_type;
  
  template <class A, class B>
  static inline
  result_type apply( const A &a, const B &b ) {
//    std::cout << "bar" << std::endl;
    
    result_type ra(Value<A>::apply(a));
    if ( ra.getStatus() == smoc_root_port_bool::IS_ENABLED )
      return result_type(ra, Value<B>::apply(b));
    else
      return ra;
  }
};

}

#endif // _INCLUDED_SMOC_ROOT_PORT_HPP
