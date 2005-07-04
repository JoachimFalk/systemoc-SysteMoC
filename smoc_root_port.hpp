// vim: set sw=2 ts=8:

#ifndef _INCLUDED_SMOC_ROOT_PORT_HPP
#define _INCLUDED_SMOC_ROOT_PORT_HPP

#include <iostream>
#include <cassert>

#include <list>

#include <systemc.h>

#include <smoc_expr.hpp>

namespace Expr { class DCommReq; }

class smoc_root_port
  : protected sc_port_base {
public:
  friend class Expr::CommSetup<Expr::DCommReq>;
  friend class Expr::CommExec<Expr::DCommReq>;
  
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

class ASTNodeCommReq: public ASTNodeTerminal {
public:
  ASTNodeCommReq() 
    : ASTNodeTerminal() {}
};

class DCommReq {
public:
  typedef bool      value_type;
  typedef DCommReq  this_type;
  
  friend class Value<this_type>;
  friend class AST<this_type>;
  friend class CommSetup<this_type>;
  friend class CommExec<this_type>;
private:
  smoc_root_port  &p;
  size_t           req;
public:
  explicit DCommReq(smoc_root_port &p, size_t req)
    : p(p), req(req) {}
};

struct CommSetup<DCommReq> {
  typedef void result_type;
  
  static inline
  result_type apply(const DCommReq &e)
    { return e.p.commSetup(e.req); }
};

struct Value<DCommReq> {
  typedef bool result_type;
  
  static inline
  result_type apply(const DCommReq &e) {
    if (e.p.availableCount() >= e.req) {
      CommSetup<DCommReq>::apply(e);
      return true;
    } else {
      return false;
    }
  }
};

struct AST<DCommReq> {
  typedef PASTNode result_type;
  
  static inline
  result_type apply(const DCommReq &e)
    { return PASTNode(new ASTNodeCommReq()); }
};

struct CommExec<DCommReq> {
  typedef void result_type;
  
  static inline
  result_type apply(const DCommReq &e)
    { return e.p.commExec(); }
};

struct D<DCommReq>: public DBase<DCommReq> {
  D(smoc_root_port &p, size_t req)
    : DBase<DCommReq>(DCommReq(p,req)) {}
};

// Make a convenient typedef for the commreq type.
struct CommReq {
  typedef D<DCommReq> type;
};

static inline
CommReq::type commreq(smoc_root_port &p, size_t req)
  { return CommReq::type(p,req); }

}
/****************************************************************************/

class smoc_commreq_number {
private:
  smoc_root_port  &p;
public:
  explicit smoc_commreq_number(smoc_root_port &p): p(p) {}
  
  Expr::CommReq::type operator >= (size_t req)
    { return Expr::commreq(p,req); }
  Expr::CommReq::type operator >  (size_t req)
    { return *this >= req+1; }
};

#endif // _INCLUDED_SMOC_ROOT_PORT_HPP
