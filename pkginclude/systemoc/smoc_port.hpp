//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
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

#ifndef _INCLUDED_SMOC_POPT_HPP
#define _INCLUDED_SMOC_POPT_HPP

#include <cosupport/commondefs.h>

#include <systemoc/smoc_config.h>

#include "smoc_expr.hpp"
#include "smoc_root_port.hpp"
#include "smoc_chan_if.hpp"
#include "smoc_event.hpp"
#include "smoc_storage.hpp"
#include "smoc_pggen.hpp"

#include <systemc.h>
#include <vector>

#include "hscd_tdsim_TraceLog.hpp"

//forward declaration
template <typename T>
class smoc_port_in;

namespace Expr {


/****************************************************************************
 * DToken is a placeholder for a token in the expression.
 */

template<typename T>
class DToken {
public:
  typedef const T    value_type;
  typedef DToken<T>  this_type;
  
  friend class Value<this_type>;
  friend class AST<this_type>;
private:
  smoc_port_in<T> &p;
  size_t           pos;
public:
  explicit DToken(smoc_port_in<T> &p, size_t pos)
    : p(p), pos(pos) {}
};

template<typename T>
struct Value<DToken<T> > {
  typedef const T result_type;
  
  static inline
  result_type apply(const DToken<T> &e)
  { return e.p[e.pos]; }
};

template<typename T>
struct AST<DToken<T> > {
  typedef PASTNode result_type;
  
  static inline
  result_type apply(const DToken<T> &e)
    { return PASTNode(new ASTNodeToken(e.p, e.pos)); }
};

template<typename T>
struct D<DToken<T> >: public DBase<DToken<T> > {
  D(smoc_port_in<T> &p, size_t pos)
    : DBase<DToken<T> >(DToken<T>(p,pos)) {}
};

// Make a convenient typedef for the token type.
template<typename T>
struct Token {
  typedef D<DToken<T> > type;
};

template <typename T>
typename Token<T>::type token(smoc_port_in<T> &p, size_t pos)
{ return typename Token<T>::type(p,pos); }

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
 * DComm represents request to consume/produce tokens
 */

template<class P, class E>
class DComm {
public:
  typedef DComm<P,E> this_type;

  friend class AST<this_type>;
  friend class CommExec<this_type>;
  friend class Value<this_type>;
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
 * DSMOCEvent represents a smoc_event guard which turns true if the event is
 * signaled
 */

class DSMOCEvent {
public:
  typedef bool       value_type;
  typedef DSMOCEvent this_type;
  
  friend class Value<this_type>;
  friend class AST<this_type>;
  friend class Sensitivity<this_type>;
private:
  smoc_event_waiter &v;
public:
  explicit DSMOCEvent(smoc_event_waiter &v): v(v) {}
};

template <>
struct Value<DSMOCEvent> {
  typedef Expr::Detail::ENABLED result_type;

  static inline
  result_type apply(const DSMOCEvent &e) {
#ifndef NDEBUG
    assert(e.v);
#endif
    return result_type();
  }
};

template <>
struct Sensitivity<DSMOCEvent> {
  typedef Detail::Process      match_type;
  
  typedef void                 result_type;
  typedef smoc_event_and_list &param1_type;
  
  static inline
  void apply(const DSMOCEvent &e, smoc_event_and_list &al) {
    al &= e.v;
//#ifdef SYSTEMOC_DEBUG
//    std::cerr << "Sensitivity<DSMOCEvent>::apply(...) al == " << al << std::endl;
//#endif
  }
};

template <>
struct AST<DSMOCEvent> {
  typedef PASTNode result_type;
  
  static inline
  result_type apply(const DSMOCEvent &e)
    { return PASTNode(new ASTNodeSMOCEvent()); }
};

template <>
struct D<DSMOCEvent>: public DBase<DSMOCEvent> {
  D(smoc_event_waiter &v): DBase<DSMOCEvent>(DSMOCEvent(v)) {}
};

// Make a convenient typedef for the placeholder type.
struct SMOCEvent { typedef D<DSMOCEvent> type; };

// smoc_event_waiter may be an event or a event list
// till-waiting for events allows for hierarchical graph scheduling
static inline
SMOCEvent::type till(smoc_event_waiter &e)
  { return SMOCEvent::type(e); }

} // namespace Expr

/****************************************************************************/

template <class P,          //parent class
	  typename T,       //interface type
	  class PARAM_TYPE  //port parameters
					>
class smoc_port_base
  : public P {
public:
  typedef T                   iface_type;
  typedef smoc_port_base<P,T,PARAM_TYPE> this_type;

  typedef typename T::access_type        ChannelAccess;
  template <class E> friend class Expr::Sensitivity;
private:
  typedef P                   base_type;
  
  iface_type  *interface;
  
  const char *if_typename() const { return typeid(iface_type).name(); }

  // called by pbind (for internal use only)
  int vbind( sc_interface& interface_ ) {
      iface_type *iface = dynamic_cast<iface_type *>( &interface_ );
      if( iface == 0 ) {
          // type mismatch
          return 2;
      }
      base_type::bind( *iface );
      return 0;
  }
  int vbind( sc_port_base& parent_ ) {
      this_type* parent = dynamic_cast<this_type*>( &parent_ );
      if( parent == 0 ) {
          // type mismatch
          return 2;
      }
      base_type::bind( *parent );
      return 0;
  }
protected:
  ChannelAccess *channelAccess;

#ifndef NDEBUG
  virtual void   setLimit(size_t l)
    {channelAccess->setLimit(l);}
  //virtual size_t getLimit() const {return channelAccess->getLimit();}
#endif

  smoc_port_base( const char* name_ )
    : base_type( name_ ), interface( NULL ) {}

  void push_interface( sc_interface *_i ) {
    assert( interface == NULL );
    interface = dynamic_cast<iface_type *>(_i);
    assert( interface != NULL );
  }

  // get the first interface without checking for nil
  sc_interface       *get_interface()       { return interface; }
  sc_interface const *get_interface() const { return interface; }

  iface_type       *operator -> () {
    if ( interface == NULL )
      this->report_error( SC_ID_GET_IF_, "port is not bound" );
    return interface;
  }
  
  iface_type const *operator -> () const {
    if ( interface == NULL )
      this->report_error( SC_ID_GET_IF_, "port is not bound" );
    return interface;
  }

public:
  virtual PARAM_TYPE params() const = 0;

};

template <typename T,
          template <typename> class R,
          class PARAM_TYPE>
class smoc_port_in_base
  : public smoc_port_base<smoc_root_port_in, smoc_chan_in_if<T,R>, PARAM_TYPE > {
public:
  typedef T            data_type;
  typedef smoc_port_in_base<data_type,R,PARAM_TYPE>      this_type;
  typedef typename this_type::iface_type    iface_type;

  typedef typename iface_type::access_type::return_type return_type;
  
  template <class E> friend class Expr::CommExec;
#ifndef NDEBUG
  template <class E> friend class Expr::CommReset;
  template <class E> friend class Expr::CommSetup;
#endif
  template <class E> friend class Expr::Value;
protected:
  typedef smoc_port_base<smoc_root_port_in, smoc_chan_in_if<T,R >, PARAM_TYPE > base_type;

  // FIXME: SystemC 2.2 HACK
  virtual int interface_count() {return 1;}

  void add_interface( sc_interface *i ) {
    this->push_interface(i);
    if (this->child == NULL)
      (*this)->registerPort(this);
  }

  void finalise(smoc_root_node *node) {
#ifdef SYSTEMOC_DEBUG
    std::cerr << "smoc_port_in_base::finalise(), name == " << this->name() << std::endl;
#endif
    // Preallocate ID
    //smoc_modes::PGWriter::getId(this);
    this->channelAccess = (*this)->getChannelAccess();
  }

#ifdef SYSTEMOC_ENABLE_VPC
  void commExec(size_t n, const smoc_ref_event_p &le)
    { return (*this)->commitRead(n, le); }
#else
  void commExec(size_t n)
    { return (*this)->commitRead(n); }
#endif
public: 
  smoc_port_in_base()
    : base_type(sc_gen_unique_name("smoc_port_in")) {}
 
  bool isInput() const { return true; }

  bool tokenIsValid(size_t i=0) const
    { return this->channelAccess->tokenIsValid(i); }
  size_t tokenId(size_t i=0) const
    { return (*this)->inTokenId() + i; }
  size_t availableCount() const
    { return (*this)->numAvailable(); }
  smoc_event &blockEvent(size_t n = MAX_TYPE(size_t))
    { return (*this)->dataAvailableEvent(n); }  

  typename Expr::PortTokens<this_type>::type getConsumableTokens()
    { return Expr::portTokens(*this); }
 
  // operator(n,m) n: How much to consume, m: How many tokens/space must be available
  typename Expr::BinOp<
    Expr::DComm<this_type,Expr::DLiteral<size_t> >,
    Expr::DBinOp<Expr::DPortTokens<this_type>,Expr::DLiteral<size_t>,Expr::DOpBinGe>,
    Expr::DOpBinLAnd>::type
  operator ()(size_t n, size_t m) {
    assert(m >= n);
    return
      Expr::comm(*this, Expr::DLiteral<size_t>(n)) &&
      getConsumableTokens() >= m;
  }
  typename Expr::BinOp<
    Expr::DComm<this_type,Expr::DLiteral<size_t> >,
    Expr::DBinOp<Expr::DPortTokens<this_type>,Expr::DLiteral<size_t>,Expr::DOpBinGe>,
    Expr::DOpBinLAnd>::type
  operator ()(size_t n)
    { return this->operator()(n,n); }
 
  void operator () ( iface_type& interface_ )
    { bind(interface_); }
  void operator () ( this_type& parent_ )
    { bind(parent_); }
};


template <typename T,                            //data type
          //template <typename, typename> class R, //ring access type
          template <typename> class R, //ring access type
          class PARAM_TYPE,                      //parameter type
          template <typename> class STORAGE_TYPE = smoc_storage_out
          >
class smoc_port_out_base
  : public smoc_port_base<smoc_root_port_out, 
                          smoc_chan_out_if<T,R,STORAGE_TYPE>, 
                          PARAM_TYPE > {
public:
  typedef T            data_type;
  typedef smoc_port_out_base<data_type,R, PARAM_TYPE, STORAGE_TYPE>      this_type;
  typedef typename this_type::iface_type    iface_type;
  
  template <class E> friend class Expr::CommExec;
#ifndef NDEBUG
  template <class E> friend class Expr::CommReset;
  template <class E> friend class Expr::CommSetup;
#endif
  template <class E> friend class Expr::Value;

  typedef typename iface_type::access_type::return_type return_type;


protected:
  typedef smoc_port_base<smoc_root_port_out, 
                         smoc_chan_out_if<T,R,STORAGE_TYPE>, 
                         PARAM_TYPE > base_type;
  // FIXME: SystemC 2.2 HACK
  virtual int interface_count() {return 1;}

  void add_interface( sc_interface *i ) {
    this->push_interface(i);
    if (this->child == NULL)
      (*this)->registerPort(this);
  }

  void finalise(smoc_root_node *node) {
    // Preallocate ID
    //smoc_modes::PGWriter::getId(this);
    this->channelAccess = (*this)->getChannelAccess();
  }

#ifdef SYSTEMOC_ENABLE_VPC
  void commExec(size_t n, const smoc_ref_event_p &le)
    { return (*this)->commitWrite(n, le); }
#else
  void commExec(size_t n)
    { return (*this)->commitWrite(n); }
#endif
public:  
  smoc_port_out_base()
    : base_type(sc_gen_unique_name("smoc_port_out")) {}
 
  bool isInput() const { return false; }
 
  size_t tokenId(size_t i=0) const
    { return (*this)->outTokenId() + i; }
  size_t availableCount() const
    { return (*this)->numFree(); }
  smoc_event &blockEvent(size_t n = MAX_TYPE(size_t))
    { return (*this)->spaceAvailableEvent(n); }

  typename Expr::PortTokens<this_type>::type getFreeSpace()
    { return Expr::portTokens<this_type>(*this); }

  // operator(n,m) n: How much to consume, m: How many tokens/space must be available
  typename Expr::BinOp<
    Expr::DComm<this_type,Expr::DLiteral<size_t> >,
    Expr::DBinOp<Expr::DPortTokens<this_type>,Expr::DLiteral<size_t>,Expr::DOpBinGe>,
    Expr::DOpBinLAnd>::type
  operator ()(size_t n, size_t m) {
    assert(m >= n);
    return
      Expr::comm(*this, Expr::DLiteral<size_t>(n)) &&
      getFreeSpace() >= m;
  }
  typename Expr::BinOp<
    Expr::DComm<this_type,Expr::DLiteral<size_t> >,
    Expr::DBinOp<Expr::DPortTokens<this_type>,Expr::DLiteral<size_t>,Expr::DOpBinGe>,
    Expr::DOpBinLAnd>::type
  operator ()(size_t n)
    { return this->operator()(n,n); }
 
  void operator () ( iface_type& interface_ )
    { bind(interface_); }
  void operator () ( this_type& parent_ )
    { bind(parent_); }
};

template <typename T>
class smoc_port_in
  : public smoc_port_in_base<T, smoc_channel_access, void > {
public:
  typedef T            data_type;
  typedef smoc_port_in<data_type>      this_type;
  typedef typename this_type::iface_type    iface_type;
  typedef typename this_type::return_type   return_type;

protected:
  typedef smoc_port_in_base<T, smoc_channel_access, void > base_type;
  
public:
  smoc_port_in(): base_type() {}

  const return_type operator[](size_t n) const {
    return (*(this->channelAccess))[n];
  }

  typename Expr::Token<T>::type getValueAt(size_t n)
    { return Expr::token<T>(*this,n); }

public:

  ///dummy function
  void params() const {
    assert(false);
  }
  
};


template <typename T>
class smoc_port_out
  : public smoc_port_out_base<T, smoc_channel_access, void > {
public:
  typedef T            data_type;
  typedef smoc_port_out<data_type>      this_type;
  typedef typename this_type::iface_type    iface_type;
  typedef typename this_type::return_type   return_type;

protected:
  typedef smoc_port_out_base<T, smoc_channel_access, void > base_type;
  
public:
  smoc_port_out(): base_type() {}

  return_type operator[](size_t n)  {
    return (*(this->channelAccess))[n];
  }

public:
  ///dummy function
  void params() const {
    assert(false);
  }
};

#endif // _INCLUDED_SMOC_POPT_HPP
