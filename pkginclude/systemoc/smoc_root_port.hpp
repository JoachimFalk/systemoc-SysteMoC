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

#include <systemoc/smoc_config.h>

#include "smoc_expr.hpp"
#include "smoc_event.hpp"

class smoc_root_node;

// forward declaration
namespace Expr { template <class E> class Value; }

namespace Expr {

/****************************************************************************
 * DPortTokens represents a count of available tokens or free space in
 * the port p
 */

//P: Port class
template<class P>
class DPortTokens {
public:
  typedef DPortTokens<P>  this_type;
  
  friend class AST<this_type>;
  template <class E> friend class Value;
  template <class E> friend class CommExec;
#ifndef NDEBUG
  template <class E> friend class CommSetup;
  template <class E> friend class CommReset;
#endif
  template <class E> friend class Sensitivity;
private:
  P      &p;
public:
  explicit DPortTokens(P &p)
    : p(p) {}
};

template<class P>
struct D<DPortTokens<P> >: public DBase<DPortTokens<P> > {
  D(P &p)
    : DBase<DPortTokens<P> >(DPortTokens<P>(p)) {}
};

template<class P>
struct AST<DPortTokens<P> > {
  typedef PASTNode result_type;
  
  static inline
  result_type apply(const DPortTokens<P> &e)
    { return PASTNode(new ASTNodePortTokens(e.p)); }
};

// Make a convenient typedef for the token type.
// P: port class
template<class P>
struct PortTokens {
  typedef D<DPortTokens<P> > type;
};

template <class P>
typename PortTokens<P>::type portTokens(P &p)
  { return typename PortTokens<P>::type(p); }

/****************************************************************************
 * DBinOp<DPortTokens<P>,E,DOpBinGe> represents a request for available/free
 * number of tokens on actor ports
 */

#ifndef NDEBUG
template <class P, class E>
struct CommReset<DBinOp<DPortTokens<P>,E,DOpBinGe> > {
  typedef void result_type;
  
  static inline
  result_type apply(const DBinOp<DPortTokens<P>,E,DOpBinGe> &e) {
# ifdef SYSTEMOC_DEBUG
    std::cerr << "CommReset<DBinOp<DPortTokens<P>,E,DOpBinGe> >"
                 "::apply(" << e.a.p << ", ... )" << std::endl;
# endif
    return e.a.p.setLimit(0);
  }
};

template <class P, class E>
struct CommSetup<DBinOp<DPortTokens<P>,E,DOpBinGe> > {
  typedef void result_type;
  
  static inline
  result_type apply(const DBinOp<DPortTokens<P>,E,DOpBinGe> &e) {
# ifdef SYSTEMOC_DEBUG
    std::cerr << "CommSetup<DBinOp<DPortTokens<P>,E,DOpBinGe> >"
                 "::apply(" << e.a.p << ", ... )" << std::endl;
# endif
    size_t req = Value<E>::apply(e.b);
# ifdef SYSTEMOC_TRACE
    TraceLog.traceCommSetup
      (dynamic_cast<smoc_root_chan *>(e.a.p.operator ->()), req);
# endif
    return e.a.p.setLimit(req);
  }
};
#endif

template <class P, typename T>
struct Sensitivity<DBinOp<DPortTokens<P>,DLiteral<T>,DOpBinGe> > {
  typedef Detail::Process      match_type;
  
  typedef void                 result_type;
  typedef smoc_event_and_list &param1_type;

  static
  void apply(const DBinOp<DPortTokens<P>,DLiteral<T>,DOpBinGe> &e,
             smoc_event_and_list &al) {
    al &= e.a.p.blockEvent(Value<DLiteral<T> >::apply(e.b));
//#ifdef SYSTEMOC_DEBUG
//  std::cerr << "Sensitivity<DBinOp<DPortTokens<P>,E,DOpBinGe> >::apply al == " << al << std::endl;
//#endif
  }
};

template <class P, class E>
struct Value<DBinOp<DPortTokens<P>,E,DOpBinGe> > {
  typedef Expr::Detail::ENABLED result_type;
  
  static inline
  result_type apply(const DBinOp<DPortTokens<P>,E,DOpBinGe> &e) {
#ifndef NDEBUG
    size_t req = Value<E>::apply(e.b);
    assert(e.a.p.availableCount() >= req);
    e.a.p.setLimit(req);
#endif
    return result_type();
  }
};

/****************************************************************************
 * DComm represents request to consume/produce tokens
 */

template<class P, class E>
class DComm {
public:
  typedef DComm<P,E> this_type;

  friend class AST<this_type>;
  friend class CommExec<this_type>;
  friend class Value<this_type>;
  friend class CommitCount<this_type>;
private:
  P &p;
  E  e;
public:
  explicit DComm(P &p, const E &e): p(p), e(e) {}
};

template<class P, class E>
struct D<DComm<P,E> >: public DBase<DComm<P,E> > {
  D(P &p, const E &e): DBase<DComm<P,E> >(DComm<P,E>(p,e)) {}
};

// Make a convenient typedef for the token type.
template<class P, class E>
struct Comm {
  typedef D<DComm<P,E> > type;
};

template <class P, class E>
typename Comm<P,E>::type comm(P &p, const E &e)
  { return typename Comm<P,E>::type(p,e); }

template<class P, class E>
struct AST<DComm<P,E> > {
  typedef PASTNode result_type;

  static inline
  result_type apply(const DComm<P,E> &e)
    { return PASTNode(new ASTNodeComm(e.p, AST<E>::apply(e.e))); }
};

template <class P, class E>
struct CommExec<DComm<P, E> > {
  typedef Detail::Process         match_type;
  typedef void                    result_type;
#ifdef SYSTEMOC_ENABLE_VPC
  typedef const smoc_ref_event_p &param1_type;
  
  static inline
  result_type apply(const DComm<P, E> &e, const smoc_ref_event_p &le) {
# ifdef SYSTEMOC_DEBUG
    std::cerr << "CommExec<DComm<P, E> >"
                 "::apply(" << e.p << ", ... )" << std::endl;
# endif
    return e.p.commExec(Value<E>::apply(e.e), le);
  }
#else
  static inline
  result_type apply(const DComm<P, E> &e) {
# ifdef SYSTEMOC_DEBUG
    std::cerr << "CommExec<DComm<P, E> >"
                 "::apply(" << e.p << ", ... )" << std::endl;
# endif
    return e.p.commExec(Value<E>::apply(e.e));
  }
#endif
};

template <class P, class E>
struct Value<DComm<P, E> > {
  typedef Expr::Detail::ENABLED result_type;

  static inline
  result_type apply(const DComm<P, E> &e) {
    return result_type();
  }
};

template <class P, class E>
struct CommitCount<DComm<P, E> > {
  typedef Detail::Process      match_type;

  typedef void                 result_type;
  typedef port_commit_map     &param1_type;

  static
  void apply(const DComm<P, E> &e, param1_type pcm) {
    size_t commitCount = Value<E>::apply(e.e);
    pcm[&(e.p)] = commitCount;
  }
};

} // namespace Expr

class smoc_root_port
// must be public inheritance for dynamic_cast in smoc_root_node to work
: public sc_port_base {
public:
  typedef smoc_root_port  this_type;

  template <class E> friend class Expr::Value;
  friend class smoc_root_node;
  friend class hscd_choice_active_node;
protected:
  smoc_root_port *parent, *child;

  typedef Expr::BinOp<
    Expr::DComm<this_type,Expr::DLiteral<size_t> >,
    Expr::DBinOp<Expr::DPortTokens<smoc_root_port>,Expr::DLiteral<size_t>,Expr::DOpBinGe>,
    Expr::DOpBinLAnd>::type   TokenGuard;

  //FIXME(MS): allow more than one "IN-Port" per Signal
  smoc_root_port(const char* name_);
public:
#ifdef SYSTEMOC_ENABLE_VPC
  virtual void commExec(size_t, const smoc_ref_event_p &) = 0;
#else
  virtual void commExec(size_t)                           = 0;
#endif
public:
  static const char* const kind_string;
  virtual const char* kind() const
    { return kind_string; }
 
#ifndef NDEBUG
  virtual void setLimit(size_t) = 0;
#endif
  virtual size_t      availableCount() const = 0;
  virtual smoc_event &blockEvent(size_t n = MAX_TYPE(size_t)) = 0;
  virtual bool        isInput() const = 0;
  bool                isOutput() const
    { return !isInput(); }

  smoc_root_port *getParentPort() const
    { return parent; }
  smoc_root_port *getChildPort() const
    { return child; }
 
  // bind interface to this port
  void bind(sc_interface &interface_ ) {
    sc_port_base::bind(interface_);
  }
  // bind parent port to this port
  void bind(this_type &parent_) {
    assert( parent == NULL && parent_.child == NULL );
    parent        = &parent_;
    parent->child = this;
    sc_port_base::bind(parent_);
  }

  // operator(n,m) n: How much to consume/produce, m: How many tokens/space must be available
  TokenGuard operator ()(size_t n, size_t m) {
    assert(m >= n);
    return
      Expr::comm(*this, Expr::DLiteral<size_t>(n)) &&
      Expr::portTokens<smoc_root_port>(*this) >= m;
  }

  void dump( std::ostream &out ) const;
  virtual ~smoc_root_port();
protected:
  /// Finalise port called by smoc_root_node::finalise
  virtual void finalise(smoc_root_node *node) = 0;
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

  Expr::PortTokens<smoc_root_port>::type getConsumableTokens()
    { return Expr::portTokens<smoc_root_port>(*this); }
};

class smoc_root_port_out
: public smoc_root_port {
public:
  smoc_root_port_out( const char* name_ )
    : smoc_root_port(name_) {}

  Expr::PortTokens<smoc_root_port>::type getFreeSpace()
    { return Expr::portTokens<smoc_root_port>(*this); }
};

#endif // _INCLUDED_SMOC_ROOT_PORT_HPP
