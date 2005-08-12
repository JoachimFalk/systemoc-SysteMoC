// vim: set sw=2 ts=8:

#ifndef _INCLUDED_SMOC_ROOT_PORT_HPP
#define _INCLUDED_SMOC_ROOT_PORT_HPP

#include <iostream>
#include <cassert>

#include <list>

#include <systemc.h>

#include <smoc_expr.hpp>

class smoc_root_port
  : protected sc_port_base {
public:
  template <class E> friend class Expr::Value;
  
  friend class smoc_root_port_bool;
  
  typedef smoc_root_port  this_type;
private:
//  bool    uplevel;
protected:
  smoc_root_port( const char* name_ )
    : sc_port_base( name_, 1 )/*, uplevel(false)*/ {}
  
  virtual void commSetup(size_t req) = 0;
  virtual void commExec()            = 0;
  virtual void reset()               = 0;
public:
  static const char* const kind_string;
  virtual const char* kind() const
    { return kind_string; }
  
  virtual sc_module *getHierarchy() const = 0;
  virtual size_t     availableCount() const = 0;
  virtual bool       isInput() const = 0;
  bool               isOutput() const
    { return !isInput(); }
  
//  bool isUplevel() const { return uplevel; }
 
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

class smoc_commnr {
public:
  typedef smoc_commnr this_type;
  
  friend class smoc_root_port_bool;
  template <typename T> friend class smoc_port_in;
  template <typename T> friend class smoc_port_out;
private:
  smoc_root_port &p;
  
  smoc_commnr(smoc_root_port &p) : p(p) {}
};

class smoc_root_port_bool {
public:
  typedef smoc_root_port_bool this_type;
  typedef void (this_type::*unspecified_bool_type)();
private:
  typedef std::list<smoc_root_port *> ports_ty;
  
//  void dummy() {};
  
  bool      v;
  ports_ty  ps;
public:
  smoc_root_port_bool( bool v = false ): v(v) {}
  smoc_root_port_bool(smoc_commnr p, size_t n)
    : v(p.p.availableCount() >= n) {
    std::cout << "was here " << v << " for " << &p.p << " !" << std::endl;
    if ( v ) {
      p.p.commSetup(n);
      ps.push_back(&p.p);
    }
  }
  smoc_root_port_bool( const this_type &a, const this_type &b )
    : v(true), ps(a.ps) {
    ps.insert(ps.end(), b.ps.begin(), b.ps.end());
  }
  
  void commExec() {
    for ( ports_ty::iterator iter = ps.begin();
          iter != ps.end();
          ++iter ) {
      (*iter)->commExec();
      (*iter)->reset();
    }
  }
  
  bool enabled() const { return v; }
  
//  operator unspecified_bool_type() const
//    { return v ? &this_type::dummy : NULL; }
  
  ~smoc_root_port_bool() {
    /* FIXME: implement reset semantic
    for ( ports_ty::iterator iter = ps.begin();
          iter != ps.end();
          ++iter ) {
      (*iter)->reset();
    }*/
  }
};

static inline
smoc_root_port_bool operator >= (smoc_commnr c, size_t n)
  { return smoc_root_port_bool(c,n); }

namespace Expr {

// NEEDED:
//  to implement short circuit boolean evaluation
//  with smoc_root_port_bool
template<class A, class B>
class DBinOp<DBinOp<DLiteral<smoc_commnr>,A,DOpBinGe>, B, DOpBinLAnd> {
public:
  typedef DBinOp<DLiteral<smoc_commnr>,A,DOpBinGe>  ExprA;
  typedef DBinOp<ExprA,B,DOpBinLAnd>                this_type;
  typedef smoc_root_port_bool                       value_type;
  
  ExprA a;
  B     b;
  
  value_type value() const {
    std::cout << "foo" << std::endl;
    
    smoc_root_port_bool retval =  Value<ExprA>::apply(a);
    
    return retval.enabled()
      ? retval && Value<B>::apply(b)
      : smoc_root_port_bool(false);
  }
public:
  DBinOp(const ExprA &a, const B &b): a(a), b(b) {}
};

}

static inline
smoc_root_port_bool operator && (const smoc_root_port_bool &a, bool b)
  { return b ? a : smoc_root_port_bool(false); }
static inline
smoc_root_port_bool operator && (const smoc_root_port_bool &a,
                                 const smoc_root_port_bool &b) {
  return a.enabled() && b.enabled()
    ? smoc_root_port_bool(a,b)
    : smoc_root_port_bool(false);
}

//static inline
//smoc_root_port_bool operator || (const smoc_root_port_bool &a, bool b)
//  { return !b || a.enabled() ? a : smoc_root_port_bool(true); }
/*
static inline
smoc_root_port_bool operator || (const smoc_root_port_bool &a,
                                 const smoc_root_port_bool &b) {
  // FIXME: implement ME !!!
  assert( "FIXME: implement ME !!!" == NULL);
}
*/

//static inline
//smoc_root_port_bool operator && (bool a, const smoc_root_port_bool &b) 
//  { return b && a; }
//static inline
//smoc_root_port_bool operator || (bool a, const smoc_root_port_bool &b)
//  { return b || a; }


#endif // _INCLUDED_SMOC_ROOT_PORT_HPP
