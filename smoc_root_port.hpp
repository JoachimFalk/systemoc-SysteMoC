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
  friend class smoc_guard_comm_request;
  
  typedef smoc_root_port  this_type;
private:
  size_t  committed;
  size_t  done;
//  size_t  maxcommittable;
  bool    uplevel;
protected:
  smoc_root_port( const char* name_ )
    : sc_port_base( name_, 1 ), uplevel(false) { reset(); }
  
  virtual void transfer() = 0;
public:
  static const char* const kind_string;
  virtual const char* kind() const
    { return kind_string; }
  
  virtual bool isInput() const = 0;
  bool isOutput() const { return !isInput(); }
  bool isUplevel() const { return uplevel; }
  
  virtual void reset() { committed = done = 0; /*maxcommittable = ~0;*/ }
  
  bool setCommittedCount( size_t _committed ) {
    assert( _committed >= committed /*&&
            _committed <= maxcommittable*/ );
    bool retval = committed != _committed;
    committed = _committed;
    return retval;
  }
//  bool setMaxCommittable( size_t _maxcommittable ) {
//    assert( _maxcommittable <= maxcommittable &&
//            _maxcommittable >= committed );
//    bool retval = maxcommittable != _maxcommittable;
//    maxcommittable = _maxcommittable;
//    return retval;
//  }
  bool canTransfer() const { return committedCount() > doneCount(); }
  operator bool() const { return !canTransfer(); }
  
  size_t doneCount() const { return done; }
  
  size_t committedCount() const { return committed; }
  virtual size_t availableCount() const = 0;
//  size_t maxCommittableCount() const { return maxcommittable; }
//  virtual size_t maxAvailableCount() const = 0;
  
  size_t incrDoneCount() { return done++; }

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
           "committed=" << p.committedCount() << ","
//           "maxcommittable=" << p.maxCommittableCount() << ","
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
private:
  smoc_root_port  &p;
  size_t           req;
public:
  explicit DCommReq(smoc_root_port &p, size_t req)
    : p(p), req(req) {}
};

struct Value<DCommReq> {
  typedef bool result_type;
  
  static inline
  result_type apply(const DCommReq &e)
    { return e.p.availableCount() >= e.req; }
};

struct AST<DCommReq> {
  typedef PASTNode result_type;
  
  static inline
  result_type apply(const DCommReq &e)
    { return PASTNode(new ASTNodeCommReq()); }
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
