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

#ifndef _INCLUDED_SMOC_POPT_HPP
#define _INCLUDED_SMOC_POPT_HPP

#include <cosupport/commondefs.h>

#include <smoc_expr.hpp>
#include <smoc_root_port.hpp>
#include <smoc_chan_if.hpp>
#include <smoc_event.hpp>
#include <smoc_storage.hpp>

#include <systemc.h>
#include <vector>

template <typename T> class smoc_port_in;
template <typename T> class smoc_port_out;

namespace Expr {

/****************************************************************************
 * DToken is a placeholder for a token in the expression.
 */

class ASTNodeToken: public ASTNodeTerminal {
private:
  std::string           type;
  const smoc_root_port &port;
  size_t                pos;
public:
  template <typename T>
  ASTNodeToken(const smoc_port_in<T> &port, size_t pos)
    : type(typeid(T).name()),
      port(port), pos(pos) {}

  const char           *getType() const { return type.c_str(); }
  const smoc_root_port *getPort() const { return &port; }
  size_t                getPos() const { return pos; }
};

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

class ASTNodePortTokens: public ASTNodeTerminal {
private:
  smoc_root_port *p;
public:
  ASTNodePortTokens(smoc_root_port *p)
    : ASTNodeTerminal(), p(p) {}
  
  const smoc_root_port *getPort() const
    { return p; }
};

template<class P>
class DPortTokens {
public:
  typedef size_t          value_type;
  typedef DPortTokens<P>  this_type;
  
  friend class AST<this_type>;
  template <class E> friend class Value;
  template <class E> friend class CommExec;
  template <class E> friend class CommSetup;
  template <class E> friend class Sensitivity;
private:
  P      &p;
public:
  explicit DPortTokens(P &p)
    : p(p) {}
};

template<class P>
struct AST<DPortTokens<P> > {
  typedef PASTNode result_type;
  
  static inline
  result_type apply(const DPortTokens<P> &e)
    { return PASTNode(new ASTNodePortTokens(&e.p)); }
};

template<class P>
struct D<DPortTokens<P> >: public DBase<DPortTokens<P> > {
  D(P &p)
    : DBase<DPortTokens<P> >(DPortTokens<P>(p)) {}
};

// Make a convenient typedef for the token type.
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

template <class P, class E>
struct CommExec<DBinOp<DPortTokens<P>,E,DOpBinGe> > {
  typedef void        result_type;
#ifdef ENABLE_SYSTEMC_VPC
  typedef const smoc_ref_event_p &param1_type;
  
  static inline
  result_type apply(const DBinOp<DPortTokens<P>,E,DOpBinGe> &e, const smoc_ref_event_p &le) {
# ifdef SYSTEMOC_DEBUG
    std::cerr << "CommExec<DBinOp<DPortTokens<P>,E,DOpBinGe> >"
                 "::apply(" << e.a.p << ", ... )" << std::endl;
# endif
    return e.a.p.commExec(le);
  }
#else
  static inline
  result_type apply(const DBinOp<DPortTokens<P>,E,DOpBinGe> &e) {
# ifdef SYSTEMOC_DEBUG
    std::cerr << "CommExec<DBinOp<DPortTokens<P>,E,DOpBinGe> >"
                 "::apply(" << e.a.p << ", ... )" << std::endl;
# endif
    return e.a.p.commExec();
  }
#endif
};

template <class P, class E>
struct CommSetup<DBinOp<DPortTokens<P>,E,DOpBinGe> > {
  typedef void result_type;
  
  static inline
  result_type apply(const DBinOp<DPortTokens<P>,E,DOpBinGe> &e) {
#ifdef SYSTEMOC_DEBUG
    std::cerr << "CommSetup<DBinOp<DPortTokens<P>,E,DOpBinGe> >"
                 "::apply(" << e.a.p << ", ... )" << std::endl;
#endif
    return e.a.p.commSetup(Value<E>::apply(e.b));
  }
};

template <class P, class E>
struct Sensitivity<DBinOp<DPortTokens<P>,E,DOpBinGe> > {
  typedef Detail::Sensitive    match_type;
  
  typedef void                 result_type;
  typedef smoc_event_and_list &param1_type;

  static
  void apply(const DBinOp<DPortTokens<P>,E,DOpBinGe> &e,
             smoc_event_and_list &al) {
    al &= e.a.p.blockEvent(Value<E>::apply(e.b));
//#ifdef SYSTEMOC_DEBUG
//    std::cerr << "Sensitivity<DBinOp<DPortTokens<P>,E,DOpBinGe> >::apply al == " << al << std::endl;
//#endif
  }
};

template <class P, class E>
struct Value<DBinOp<DPortTokens<P>,E,DOpBinGe> > {
  typedef bool result_type;
  
  static inline
  result_type apply(const DBinOp<DPortTokens<P>,E,DOpBinGe> &e) {
    size_t req = Value<E>::apply(e.b);
    assert(e.a.p.availableCount() >= req);
    e.a.p.commSetup(req);
    return true;
  }
/*{
    size_t req = Value<E>::apply(e.b);
#ifdef SYSTEMOC_DEBUG
    std::cerr << "Value<DBinOp<DPortTokens<P>,E,DOpBinGe> >::apply "
      <<  e.a.p.availableCount() << " >= " << req << std::endl;
#endif
    if (e.a.p.availableCount() >= req) {
      e.a.p.commSetup(req);
      return true;
    } else {
      e.a.p.blockEvent().reset();
      return false;
    }
  } */
};

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
  friend class Sensitivity<this_type>;
private:
  value_type &v;
public:
  explicit DSMOCEvent(value_type &v): v(v) {}
};

template <>
struct Value<DSMOCEvent> {
  typedef bool result_type;
  
  static inline
  result_type apply(const DSMOCEvent &e)
    { return e.v; }
};

template <>
struct Sensitivity<DSMOCEvent> {
  typedef Detail::Sensitive    match_type;
  
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
  D(smoc_event &v): DBase<DSMOCEvent>(DSMOCEvent(v)) {}
};

// Make a convenient typedef for the placeholder type.
struct SMOCEvent { typedef D<DSMOCEvent> type; };

static inline
SMOCEvent::type till(smoc_event &e)
  { return SMOCEvent::type(e); }

} // namespace Expr

/****************************************************************************/

template <class P, typename T>
class smoc_port_base
  : public P {
public:
  typedef T                   iface_type;
  typedef smoc_port_base<P,T> this_type;

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
};

template <typename T>
class smoc_port_in
//: public smoc_port_storage_in<T> {
: public smoc_port_base<smoc_root_port_in, smoc_chan_in_if<T> >,
  public smoc_ring_access<
    typename smoc_storage_in<T>::storage_type,
    typename smoc_storage_in<T>::return_type>
{
public:
  typedef T						    data_type;
  typedef smoc_port_in<data_type>			    this_type;
  typedef typename this_type::iface_type		    iface_type;
  typedef typename smoc_storage_in<data_type>::storage_type storage_type;
  typedef typename smoc_storage_in<data_type>::return_type  return_type;
  typedef smoc_ring_access<storage_type, return_type>	    ring_type;
  
  template <class E> friend class Expr::CommExec;
  template <class E> friend class Expr::CommSetup;
  template <class E> friend class Expr::Value;
protected:
  typedef smoc_port_base<smoc_root_port_in, smoc_chan_in_if<T> > base_type;

  void add_interface( sc_interface *i ) {
    this->push_interface(i);
    if (this->child == NULL)
      (*this)->addPort(this);
  }

  bool peerIsV1() const
    { return (*this)->portOutIsV1(); }

  void commSetup(size_t req) {
    static_cast<ring_type &>(*this) =
      (*this)->commSetupIn(req);
  }
#ifdef ENABLE_SYSTEMC_VPC
  void commExec(const smoc_ref_event_p &le) { (*this)->commExecIn(*this, le); }
#else
  void commExec() { (*this)->commExecIn(*this); }
#endif
  void reset() { ring_type::reset(); }
public:
//void transferIn( const T *in ) { /*storagePushBack(in);*/ incrDoneCount(); }
//public:
  smoc_port_in(): base_type(sc_gen_unique_name("smoc_port_in")) {}
  
  bool isInput() const { return true; }
  
  size_t availableCount() const
    { return (*this)->committedOutCount(); }
  smoc_event &blockEvent(size_t n = MAX_TYPE(size_t))
    { return (*this)->blockEventOut(n); }
  
  typename Expr::Token<T>::type getValueAt(size_t n)
    { return Expr::token(*this,n); }
  typename Expr::PortTokens<this_type>::type getAvailableTokens()
    { return Expr::portTokens(*this); }
  
  Expr::D<Expr::DBinOp<Expr::DPortTokens<this_type>,
                       Expr::DLiteral<size_t>,
                       Expr::DOpBinGe> >
  operator ()( size_t n )
    { return getAvailableTokens() >= n; }
  
  void operator () ( iface_type& interface_ )
    { interface_.is_v1_in_port = this->is_smoc_v1_port; bind(interface_); }
  void operator () ( this_type& parent_ )
    { bind(parent_); }
};

template <typename T>
class smoc_port_out
//: public smoc_port_storage_out<T> {
: public smoc_port_base<smoc_root_port_out, smoc_chan_out_if<T> >,
  public smoc_ring_access<
    typename smoc_storage_out<T>::storage_type,
    typename smoc_storage_out<T>::return_type>
{
public:
  typedef T						     data_type;
  typedef smoc_port_out<data_type>			     this_type;
  typedef typename this_type::iface_type		     iface_type;
  typedef typename smoc_storage_out<data_type>::storage_type storage_type;
  typedef typename smoc_storage_out<data_type>::return_type  return_type;
  typedef smoc_ring_access<storage_type, return_type>	     ring_type;
  
  template <class E> friend class Expr::CommExec;
  template <class E> friend class Expr::CommSetup;
  template <class E> friend class Expr::Value;
protected:
  typedef smoc_port_base<smoc_root_port_out, smoc_chan_out_if<T> > base_type;

  void add_interface( sc_interface *i ) {
    this->push_interface(i);
    if (this->child == NULL)
      (*this)->addPort(this);
  }

  bool peerIsV1() const
    { return (*this)->portInIsV1(); }

  void commSetup(size_t req) {
    static_cast<ring_type &>(*this) =
      (*this)->commSetupOut(req);
  }
#ifdef ENABLE_SYSTEMC_VPC
  void commExec(const smoc_ref_event_p &le) { (*this)->commExecOut(*this, le); }
#else
  void commExec() { (*this)->commExecOut(*this); }
#endif
  void reset() { ring_type::reset(); }
public:
//  const T *transferOut( void ) { /*return storageElement(*/;incrDoneCount()/*)*/; return NULL; }
//public:
  smoc_port_out(): base_type(sc_gen_unique_name("smoc_port_out")) {}
  
  bool isInput() const { return false; }
  
  size_t availableCount() const
    { return (*this)->committedInCount(); }
  smoc_event &blockEvent(size_t n = MAX_TYPE(size_t))
    { return (*this)->blockEventIn(n); }
  
  typename Expr::PortTokens<this_type>::type getAvailableSpace()
    { return Expr::portTokens<this_type>(*this); }
  
  Expr::D<Expr::DBinOp<Expr::DPortTokens<this_type>,
                       Expr::DLiteral<size_t>,
                       Expr::DOpBinGe> >
  operator ()( size_t n )
    { return getAvailableSpace() >= n; }
  
  void operator () ( iface_type& interface_ )
    { interface_.is_v1_out_port = this->is_smoc_v1_port; bind(interface_); }
  void operator () ( this_type& parent_ )
    { bind(parent_); }
};

#endif // _INCLUDED_SMOC_POPT_HPP
