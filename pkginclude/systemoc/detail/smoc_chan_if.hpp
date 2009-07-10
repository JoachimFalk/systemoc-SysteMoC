//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2009 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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

#ifndef _INCLUDED_DETAIL_SMOC_CHAN_IF_HPP
#define _INCLUDED_DETAIL_SMOC_CHAN_IF_HPP

#include <list>

#include <systemc.h>

#include <boost/noncopyable.hpp>

#include <CoSupport/SystemC/ChannelModificationListener.hpp> 

#include <systemoc/smoc_config.h>

#include <smoc/smoc_simulation_ctx.hpp>
#include "smoc_storage.hpp"
#include "smoc_sysc_port.hpp"
#include "smoc_event_decls.hpp"
#include "../smoc_expr.hpp"
#include "smoc_debug_stream.hpp"

#ifdef SYSTEMOC_ENABLE_VPC
// forward declaration
namespace SystemC_VPC {
  class FastLink;
}
#endif // SYSTEMOC_ENABLE_VPC

// forward declaration
namespace Expr { template <class E> class Value; }

namespace Expr {

/****************************************************************************
 * DPortTokens represents a count of available tokens or free space in
 * the port p
 */

//CI: Port class
template<class CI>
class DPortTokens {
public:
  typedef DPortTokens<CI>  this_type;

  template <class E> friend class VisitorApplication;
  template <class E> friend class CommExec;
#if defined(SYSTEMOC_ENABLE_DEBUG)
  template <class E> friend class CommSetup;
  template <class E> friend class CommReset;
#endif
  template <class E> friend class Sensitivity;
  template <class E> friend class Value;
private:
  smoc_sysc_port &p;

  CI &getCI() const {
    return *static_cast<CI *>(
      const_cast<smoc_port_base_if *>(p.get_interface()));
  }
public:
  explicit DPortTokens(smoc_sysc_port &p)
    : p(p) {}
};

template<class CI>
struct D<DPortTokens<CI> >: public DBase<DPortTokens<CI> > {
  D(smoc_sysc_port &p)
    : DBase<DPortTokens<CI> >(DPortTokens<CI>(p)) {}
};

template<class CI>
class VisitorApplication<DPortTokens<CI> > {
public:
  typedef void                      *result_type;
  typedef Detail::ExprVisitor<void> &param1_type;

  static inline
  result_type apply(const DPortTokens<CI> &e, param1_type p)
    { return p.visitPortTokens(e.p); }
};

// Make a convenient typedef for the token type.
// CI: port class
template<class CI>
struct PortTokens {
  typedef D<DPortTokens<CI> > type;
};

template <class P>
typename PortTokens<typename P::chan_base_type>::type portTokens(P &p)
  { return typename PortTokens<typename P::chan_base_type>::type(p); }

/****************************************************************************
 * DBinOp<DPortTokens<CI>,E,OpBinT::Ge> represents a request for available/free
 * number of tokens on actor ports
 */

#if defined(SYSTEMOC_ENABLE_DEBUG)
template <class CI, class E>
struct CommReset<DBinOp<DPortTokens<CI>,E,Expr::OpBinT::Ge> >
{
  typedef void result_type;

  static inline
  result_type apply(const DBinOp<DPortTokens<CI>,E,Expr::OpBinT::Ge> &e)
  {
# ifdef SYSTEMOC_DEBUG
    outDbg << EXPR << "CommReset<DBinOp<DPortTokens<CI>,E,OpBinT::Ge> >"
                 "::apply(" << e.a.p.name() << ", ... )" << std::endl << INFO;
# endif
    return e.a.p.portAccess->setLimit(0);
  }
};

template <class CI, class E>
struct CommSetup<DBinOp<DPortTokens<CI>,E,Expr::OpBinT::Ge> >
{
  typedef void result_type;

  static inline
  result_type apply(const DBinOp<DPortTokens<CI>,E,Expr::OpBinT::Ge> &e)
  {
# ifdef SYSTEMOC_DEBUG
    outDbg << EXPR << "CommSetup<DBinOp<DPortTokens<CI>,E,OpBinT::Ge> >"
                 "::apply(" << e.a.p.name() << ", ... )" << std::endl << INFO;
# endif
    size_t req = Value<E>::apply(e.b);
# ifdef SYSTEMOC_TRACE
    TraceLog.traceCommSetup
      (dynamic_cast<smoc_root_chan *>(e.a.p.operator ->()), req);
# endif
    return e.a.p.portAccess->setLimit(req);
  }
};
#endif

template <class CI, typename T>
struct Sensitivity<DBinOp<DPortTokens<CI>,DLiteral<T>,Expr::OpBinT::Ge> >
{
  typedef Detail::Process      match_type;

  typedef void                 result_type;
  typedef smoc_event_and_list &param1_type;

  static
  void apply(const DBinOp<DPortTokens<CI>,DLiteral<T>,Expr::OpBinT::Ge> &e,
             smoc_event_and_list &al)
  {
    al &= e.a.getCI().blockEvent(Value<DLiteral<T> >::apply(e.b));
//#ifdef SYSTEMOC_DEBUG
//  outDbg << EXPR << "Sensitivity<DBinOp<DPortTokens<CI>,E,OpBinT::Ge> >::apply al == " << al << std::endl << INFO;
//#endif
  }
};

template <class CI, class E>
struct Value<DBinOp<DPortTokens<CI>,E,Expr::OpBinT::Ge> >
{
  typedef Expr::Detail::ENABLED result_type;

  static inline
  result_type apply(const DBinOp<DPortTokens<CI>,E,Expr::OpBinT::Ge> &e)
  {
#if defined(SYSTEMOC_ENABLE_DEBUG)
    size_t req = Value<E>::apply(e.b);
    assert(e.a.getCI().availableCount() >= req);
    // WHY is this needed? This should already be done by CommSetup!
    e.a.p.portAccess->setLimit(req); 
#endif
    return result_type();
  }
};

/****************************************************************************
 * DComm represents request to consume/produce tokens
 */

template<class CI, class E>
class DComm {
public:
  typedef DComm<CI,E> this_type;

  friend class VisitorApplication<this_type>;
  friend class CommExec<this_type>;
#if defined(SYSTEMOC_ENABLE_DEBUG)
  friend class CommSetup<this_type>;
  friend class CommReset<this_type>;
#endif
  friend class Sensitivity<this_type>;
  friend class Value<this_type>;
private:
  smoc_sysc_port &p;
  E               e;

  CI &getCI() const {
    return *static_cast<CI *>(
      const_cast<smoc_port_base_if *>(p.get_interface()));
  }
public:
  explicit DComm(smoc_sysc_port &p, const E &e): p(p), e(e) {}
};

template<class CI, class E>
struct D<DComm<CI,E> >: public DBase<DComm<CI,E> > {
  D(smoc_sysc_port &p, const E &e): DBase<DComm<CI,E> >(DComm<CI,E>(p,e)) {}
};

// Make a convenient typedef for the token type.
template<class CI, class E>
struct Comm {
  typedef D<DComm<CI,E> > type;
};

template <class P, class E>
typename Comm<typename P::chan_base_type,E>::type comm(P &p, const E &e)
  { return typename Comm<typename P::chan_base_type,E>::type(p,e); }

template<class CI, class E>
class VisitorApplication<DComm<CI,E> > {
public:
  typedef void                      *result_type;
  typedef Detail::ExprVisitor<void> &param1_type;

  static inline
  result_type apply(const DComm<CI,E> &e, param1_type p)
    { return p.visitComm(e.p, boost::bind(VisitorApplication<E>::apply, e.e, _1)); }
};

template <class CI, class E>
struct CommExec<DComm<CI, E> > {
  typedef Detail::Process         match_type;
  typedef void                    result_type;
#ifdef SYSTEMOC_ENABLE_VPC
  typedef const smoc_ref_event_p &param1_type;
  typedef const smoc_ref_event_p &param2_type;

  static inline
  result_type apply(const DComm<CI, E> &e,
      const smoc_ref_event_p &diiEvent,
      const smoc_ref_event_p &latEvent) {
# ifdef SYSTEMOC_DEBUG
    outDbg << EXPR << "CommExec<DComm<CI, E> >"
                 "::apply(" << e.p.name() << ", ... )" << std::endl << INFO;
# endif
    return e.getCI().commExec(Value<E>::apply(e.e), diiEvent, latEvent);
  }
#else
  static inline
  result_type apply(const DComm<CI, E> &e) {
# ifdef SYSTEMOC_DEBUG
    outDbg << EXPR << "CommExec<DComm<CI, E> >"
                 "::apply(" << e.p.name() << ", ... )" << std::endl << INFO;
# endif
    return e.getCI().commExec(Value<E>::apply(e.e));
  }
#endif
};

template <class CI, class E>
struct Value<DComm<CI, E> > {
  typedef Expr::Detail::ENABLED result_type;

  static inline
  result_type apply(const DComm<CI, E> &e) {
    return result_type();
  }
};

} // namespace Expr


class smoc_port_in_base_if
: public smoc_port_base_if {
  typedef smoc_port_in_base_if this_type;

  friend class smoc_graph_synth;
  friend class smoc_multicast_sr_signal_chan_base;
  friend class smoc_multireader_fifo_chan_base;
  friend class smoc_signal_chan_base;

  template<class,class> friend class smoc_chan_adapter;

  // Friends needed for guard evaluation
  template <class E> friend class Expr::Sensitivity;
  template <class E> friend class Expr::CommExec;
#if defined(SYSTEMOC_ENABLE_DEBUG)
  template <class E> friend class Expr::CommReset;
  template <class E> friend class Expr::CommSetup;
#endif
  template <class E> friend class Expr::Value;
public:
  template <class PORT>
  class PortMixin {
  private:
    PORT       *getImpl()
      { return static_cast<PORT       *>(this); }
    PORT const *getImpl() const
      { return static_cast<PORT const *>(this); }
  public:
    typedef this_type chan_base_type;

    typedef Expr::BinOp<
      Expr::DComm<this_type,Expr::DLiteral<size_t> >,
      Expr::DBinOp<Expr::DPortTokens<this_type>,Expr::DLiteral<size_t>,Expr::OpBinT::Ge>,
      Expr::OpBinT::LAnd>::type                 CommAndPortTokensGuard;
    typedef Expr::PortTokens<this_type>::type PortTokensGuard;
  public:
    // operator(n,m) n: How many tokens to consume, m: How many tokens must be available
    CommAndPortTokensGuard communicate(size_t n, size_t m) {
      assert(m >= n);
      return
        Expr::comm(*getImpl(), Expr::DLiteral<size_t>(n)) &&
        Expr::portTokens(*getImpl()) >= m;
    }

    PortTokensGuard getConsumableTokens()
      { return Expr::portTokens(*getImpl()); }
  };
protected:
  // constructor
  smoc_port_in_base_if() {}
  
#ifdef SYSTEMOC_ENABLE_VPC
  virtual void        commitRead(size_t consume, const smoc_ref_event_p &) = 0;
#else
  virtual void        commitRead(size_t consume) = 0;
#endif
  virtual smoc_event &dataAvailableEvent(size_t n) = 0;
  virtual size_t      numAvailable() const = 0;

  smoc_event &blockEvent(size_t n)
    { return this->dataAvailableEvent(n); }  
  size_t availableCount() const
    { return this->numAvailable(); }
#ifdef SYSTEMOC_ENABLE_VPC
  void commExec(
      size_t n,
      const smoc_ref_event_p &diiEvent,
      const smoc_ref_event_p &latEvent)
    { return this->commitRead(n, diiEvent); }
#else
  void commExec(size_t n)
    { return this->commitRead(n); }
#endif

  /// @brief More tokens available
  virtual void moreData() {}
  /// @brief Less tokens available
  virtual void lessData() {}
  /// @brief Reset
  virtual void reset() {}

public:
  virtual size_t      inTokenId() const = 0;

  virtual ~smoc_port_in_base_if() {}
};

class smoc_port_out_base_if
: public smoc_port_base_if {
  typedef smoc_port_out_base_if this_type;

  friend class smoc_graph_synth;
  friend class smoc_multicast_sr_signal_chan_base;
  friend class smoc_multireader_fifo_chan_base;
  friend class smoc_signal_chan_base;

  template<class,class> friend class smoc_chan_adapter;

  // Friends needed for guard evaluation
  template <class E> friend class Expr::Sensitivity;
  template <class E> friend class Expr::CommExec;
#if defined(SYSTEMOC_ENABLE_DEBUG)
  template <class E> friend class Expr::CommReset;
  template <class E> friend class Expr::CommSetup;
#endif
  template <class E> friend class Expr::Value;
public:
  template <class PORT>
  class PortMixin {
  private:
    PORT       *getImpl()
      { return static_cast<PORT       *>(this); }
    PORT const *getImpl() const
      { return static_cast<PORT const *>(this); }
  public:
    typedef this_type chan_base_type;

    typedef Expr::BinOp<
      Expr::DComm<this_type,Expr::DLiteral<size_t> >,
      Expr::DBinOp<Expr::DPortTokens<this_type>,Expr::DLiteral<size_t>,Expr::OpBinT::Ge>,
      Expr::OpBinT::LAnd>::type                 CommAndPortTokensGuard;
    typedef Expr::PortTokens<this_type>::type PortTokensGuard;
  public:
    // operator(n,m) n: How many tokens to produce, m: How much space must be available
    CommAndPortTokensGuard communicate(size_t n, size_t m) {
      assert(m >= n);
      return
        Expr::comm(*getImpl(), Expr::DLiteral<size_t>(n)) &&
        Expr::portTokens(*getImpl()) >= m;
    }

    PortTokensGuard getFreeSpace()
      { return Expr::portTokens(*getImpl()); }
  };
protected:
  // constructor
  smoc_port_out_base_if() {}

#ifdef SYSTEMOC_ENABLE_VPC
  virtual void        commitWrite(size_t produce, const smoc_ref_event_p &) = 0;
#else
  virtual void        commitWrite(size_t produce) = 0;
#endif
  virtual smoc_event &spaceAvailableEvent(size_t n) = 0;
  virtual size_t      numFree() const = 0;

  smoc_event &blockEvent(size_t n)
    { return this->spaceAvailableEvent(n); }  
  size_t availableCount() const
    { return this->numFree(); }
#ifdef SYSTEMOC_ENABLE_VPC
  void commExec(
      size_t n,
      const smoc_ref_event_p &diiEvent,
      const smoc_ref_event_p &latEvent)
    { return this->commitWrite(n, latEvent); }
#else
  void commExec(size_t n)
    { return this->commitWrite(n); }
#endif
  
  /// @brief More free space available
  virtual void moreSpace() {}
  /// @brief Less free space available
  virtual void lessSpace() {}
  /// @brief Reset
  virtual void reset() {}

public:
  virtual size_t      outTokenId() const = 0;

  virtual ~smoc_port_out_base_if() {}
};

const sc_event& smoc_default_event_abort();

template <
  typename T,                                     // data type
  template <typename> class R>                    // ring access type
class smoc_port_in_if
: public smoc_port_in_base_if {
  typedef smoc_port_in_if<T,R>                  this_type;
public:
  typedef T                                     data_type;
  typedef R<
    typename smoc_storage_in<T>::return_type>   access_in_type;
  typedef access_in_type                        access_type;
  typedef this_type                             iface_type;
protected:
  // constructor
  smoc_port_in_if() {}

  virtual access_type *getReadPortAccess() = 0;
  
public:
  access_type *getChannelAccess()
    { return getReadPortAccess(); }

private:
  // disabled
  const sc_event& default_event() const
    { return smoc_default_event_abort(); }
};

template <
  typename T,                                     // data type
  template <typename> class R,                    // ring access type
  template <typename> class S = smoc_storage_out> // smoc_storage
class smoc_port_out_if
: public smoc_port_out_base_if {
  typedef smoc_port_out_if<T,R,S> this_type;
public:
  typedef T                       data_type;
  typedef R<
    typename S<T>::return_type>   access_out_type;
  typedef access_out_type         access_type;
  typedef this_type               iface_type;
protected:
  // constructor
  smoc_port_out_if() {}

  virtual access_type *getWritePortAccess() = 0;

public:
  access_type *getChannelAccess()
    { return getWritePortAccess(); }

private:
  // disabled
  const sc_event& default_event() const
    { return smoc_default_event_abort(); }
};

/**
 * This interface is obsolete and only used by WSDF.
 * Do not create new implementations based on this
 * interface!
 *
 * FIXME: Could now be also broken due to implementation
 * which adheres not to the standard!!!
 */
template <
  typename T_data_type,                           // data type
  template <typename> class R_IN,                 // ring access type for input
  template <typename> class R_OUT,                // ring access type for output
  template <typename> class S = smoc_storage_out> // smoc_storage for output
class smoc_chan_if
: public smoc_port_in_if<T_data_type, R_IN>,
  public smoc_port_out_if<T_data_type, R_OUT, S>
{
public:
  /// typedefs
  typedef smoc_port_in_if<T_data_type, R_IN>      if_1_type;
  typedef smoc_port_out_if<T_data_type, R_OUT, S> if_2_type;
  typedef smoc_chan_if<T_data_type,R_IN,R_OUT,S>  this_type;
  typedef T_data_type                             data_type;
private:
  // disabled
  const sc_event& default_event() const
    { return smoc_default_event_abort(); }
};

#endif // _INCLUDED_DETAIL_SMOC_CHAN_IF_HPP
