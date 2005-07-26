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
  template <class E> friend class Expr::CommSetup;
  template <class E> friend class Expr::CommExec;
  friend class smoc_commreq_number;
  
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
  
  friend class Value<this_type>;
  friend class AST<this_type>;
  template <class E> friend class CommSetup;
  template <class E> friend class CommExec;
private:
  smoc_root_port  &p;
public:
  explicit DCommNr(smoc_root_port &p)
    : p(p) {}
};

template <class B>
struct CommSetup<DBinOp<DCommNr, B, DOpGe> > {
  typedef void result_type;
  
  static inline
  result_type apply(const DBinOp<DCommNr, B, DOpGe> &e)
    { return e.a.p.commSetup(Value<B>::apply(e.b)); }
};

struct Value<DCommNr> {
  typedef DCommNr::value_type result_type;
  
  static inline
  result_type apply(const DCommNr &e)
    { return e.p.availableCount(); }
};

struct AST<DCommNr> {
  typedef PASTNode result_type;
  
  static inline
  result_type apply(const DCommNr &e)
    { return PASTNode(new ASTNodeCommNr()); }
};

template <class B>
struct CommExec<DBinOp<DCommNr, B, DOpGe> > {
  typedef void result_type;
  
  static inline
  result_type apply(const DBinOp<DCommNr, B, DOpGe> &e)
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

template <class B, OpType Op>
class DOpExecute<DCommNr,B,Op> {};

template <class B>
class DOpExecute<DCommNr,B,DOpGt> {
public:
  typedef DBinOp<DCommNr,B,DOpGt>                 ExprT;
  typedef D<ExprT>                                result_type;
  
  static inline
  result_type apply(const DCommNr &a, const B &b)
    { return result_type(ExprT(a,b)); }
};
template <class TB>
class DOpExecute<DCommNr,DLiteral<TB>,DOpGt> {
public:
  typedef DBinOp<DCommNr,DLiteral<size_t>,DOpGt>  ExprT;
  typedef D<ExprT>                                result_type;
  
  static inline
  result_type apply(const DCommNr &a, const DLiteral<size_t> &b)
    { return result_type(ExprT(a,b)); }
};
template <class B>
class DOpExecute<DCommNr,B,DOpGe> {
public:
  typedef DBinOp<DCommNr,B,DOpGe>                 ExprT;
  typedef D<ExprT>                                result_type;
  
  static inline
  result_type apply(const DCommNr &a, const B &b)
    { return result_type(ExprT(a,b)); }
};
template <class TB>
class DOpExecute<DCommNr,DLiteral<TB>,DOpGe> {
public:
  typedef DBinOp<DCommNr,DLiteral<size_t>,DOpGe>  ExprT;
  typedef D<ExprT>                                result_type;
  
  static inline
  result_type apply(const DCommNr &a, const DLiteral<size_t> &b)
    { return result_type(ExprT(a,b)); }
};

}
/****************************************************************************/

class smoc_commreq_number {
private:
  smoc_root_port  &p;
public:
  explicit smoc_commreq_number(smoc_root_port &p): p(p) {}
  
  bool operator >= (size_t req)
    { return p.availableCount() >= req; }
  bool operator >  (size_t req)
    { return *this >= req+1; }
};

#endif // _INCLUDED_SMOC_ROOT_PORT_HPP
