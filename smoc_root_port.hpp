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
  template <class E> friend class Expr::CommSetup;
  template <class E> friend class Expr::CommExec;
  
  typedef smoc_root_port  this_type;
private:
  bool    uplevel;
protected:
  smoc_root_port( const char* name_ )
    : sc_port_base( name_, 1 ), uplevel(false) {}
  
  virtual void commSetup(size_t req) = 0;
  virtual void commExec()            = 0;
public:
  static const char* const kind_string;
  virtual const char* kind() const
    { return kind_string; }
  
  virtual bool isInput() const = 0;
  bool isOutput() const { return !isInput(); }
  bool isUplevel() const { return uplevel; }
  
  virtual size_t availableCount()      const = 0;
  
  // bind interface to this port
  void bind( sc_interface& interface_ ) { sc_port_base::bind(interface_); }
  // bind parent port to this port
  void bind( this_type &parent_ ) { uplevel = true; sc_port_base::bind(parent_); }
private:
  // disabled
  smoc_root_port( const this_type & );
  this_type& operator = ( const this_type & );
};

static inline
std::ostream &operator <<( std::ostream &out, const smoc_root_port &p ) {
  out << "port(" << &p << ","
           "uplevel=" << p.isUplevel() << ","
//           "committed=" << p.committedCount() << ","
           "available=" << p.availableCount() << ")";
  return out;
}

typedef std::list<smoc_root_port *> smoc_port_list;

/****************************************************************************
 * DExprCommReq is a placeholder for a Communication Request, either available
 * tokens in an input port or free space in an output port.
 */

namespace Expr {

class ASTNodeCommNr: public ASTNodeTerminal {
public:
  ASTNodeCommNr() 
    : ASTNodeTerminal() {}
};

class DCommNr {
public:
  typedef size_t   value_type;
  typedef DCommNr  this_type;
  
  friend class AST<this_type>;
  template <class E> friend class Value;
  template <class E> friend class CommSetup;
  template <class E> friend class CommExec;
private:
  smoc_root_port  &p;
public:
  explicit DCommNr(smoc_root_port &p)
    : p(p) {}
};

struct AST<DCommNr> {
  typedef PASTNode result_type;
  
  static inline
  result_type apply(const DCommNr &e)
    { return PASTNode(new ASTNodeCommNr()); }
};

template <class B>
struct Value<DBinOp<DCommNr, B, DOpBinGe> > {
  typedef bool result_type;
  
  static inline
  result_type apply(const DBinOp<DCommNr, B, DOpBinGe> &e) {
    size_t n      = Value<B>::apply(e.b);
    bool   retval = e.a.p.availableCount() >= n;
    
    if ( retval )
      e.a.p.commSetup(n);
    return retval;
  }
};

template <class B>
struct CommSetup<DBinOp<DCommNr, B, DOpBinGe> > {
  typedef void result_type;
  
  static inline
  result_type apply(const DBinOp<DCommNr, B, DOpBinGe> &e) {}
//    { return e.a.p.commSetup(Value<B>::apply(e.b)); }
};

template <class B>
struct CommExec<DBinOp<DCommNr, B, DOpBinGe> > {
  typedef void result_type;
  
  static inline
  result_type apply(const DBinOp<DCommNr, B, DOpBinGe> &e)
    { return e.a.p.commExec(); }
};

struct D<DCommNr>: public DBase<DCommNr> {
  D(smoc_root_port &p)
    : DBase<DCommNr>(DCommNr(p)) {}
};

// Make a convenient typedef for the CommNr type.
struct CommNr {
  typedef D<DCommNr> type;
};

static inline
CommNr::type commnr(smoc_root_port &p)
  { return CommNr::type(p); }

template <class B, OpBinT Op>
class DOpBinExecute<DCommNr,B,Op> {};

template <class B>
class DOpBinExecute<DCommNr,B,DOpBinGt> {
public:
  typedef DBinOp<DCommNr,B,DOpBinGt>                 ExprT;
  typedef D<ExprT>                                   result_type;
  
  static inline
  result_type apply(const DCommNr &a, const B &b)
    { return result_type(ExprT(a,b)); }
};
template <class TB>
class DOpBinExecute<DCommNr,DLiteral<TB>,DOpBinGt> {
public:
  typedef DBinOp<DCommNr,DLiteral<size_t>,DOpBinGt>  ExprT;
  typedef D<ExprT>                                   result_type;
  
  static inline
  result_type apply(const DCommNr &a, const DLiteral<size_t> &b)
    { return result_type(ExprT(a,b)); }
};
template <class B>
class DOpBinExecute<DCommNr,B,DOpBinGe> {
public:
  typedef DBinOp<DCommNr,B,DOpBinGe>                 ExprT;
  typedef D<ExprT>                                   result_type;
  
  static inline
  result_type apply(const DCommNr &a, const B &b)
    { return result_type(ExprT(a,b)); }
};
template <class TB>
class DOpBinExecute<DCommNr,DLiteral<TB>,DOpBinGe> {
public:
  typedef DBinOp<DCommNr,DLiteral<size_t>,DOpBinGe>  ExprT;
  typedef D<ExprT>                                   result_type;
  
  static inline
  result_type apply(const DCommNr &a, const DLiteral<size_t> &b)
    { return result_type(ExprT(a,b)); }
};

}
/****************************************************************************/

#endif // _INCLUDED_SMOC_ROOT_PORT_HPP
