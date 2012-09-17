//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2012-2012 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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

#ifndef _INCLUDED_SMOC_DETAIL_PORTIOBASEIF_HPP
#define _INCLUDED_SMOC_DETAIL_PORTIOBASEIF_HPP

#include "PortBaseIf.hpp"

#include "../smoc_expr.hpp"

class smoc_graph_synth;
class smoc_multicast_sr_signal_chan_base;
class smoc_multireader_fifo_chan_base;
class smoc_signal_chan_base;

template<class, class> class smoc_chan_adapter;
template<class, class> class smoc_multiplex_fifo_chan;

namespace smoc { namespace Detail {

template<class IFaceImpl>
class ChanAdapterMid;

class PortInBaseIf: public PortBaseIf {
  typedef PortInBaseIf this_type;

  friend class ::smoc_graph_synth;
  friend class ::smoc_multicast_sr_signal_chan_base;
  friend class ::smoc_multireader_fifo_chan_base;
  friend class ::smoc_signal_chan_base;
  template<class,class> friend class ::smoc_multiplex_fifo_chan;
  template<class,class> friend class ::smoc_chan_adapter;

  template<class>       friend class ChanAdapterMid;

  // Friends needed for guard evaluation
  template <class E> friend class Expr::Sensitivity;
  template <class E> friend class Expr::CommExec;
#if defined(SYSTEMOC_ENABLE_DEBUG)
  template <class E> friend class Expr::CommReset;
  template <class E> friend class Expr::CommSetup;
#endif
  template <class E> friend class Expr::Value;
public:
  template <class PORT, class IFACE>
  class PortMixin {
  private:
    PORT       *getImpl()
      { return static_cast<PORT       *>(this); }
    PORT const *getImpl() const
      { return static_cast<PORT const *>(this); }
  public:
    typedef this_type                         chan_base_type;
    typedef IFACE                             iface_type;
    typedef typename iface_type::access_type  access_type;
    typedef typename iface_type::data_type    data_type;
    typedef typename access_type::return_type return_type;

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
        Expr::comm(*getImpl(),
                   Expr::DLiteral<size_t>(n),
                   Expr::DLiteral<size_t>(m))
        && //FIXME: Expr::comm knows n and m -> should remove Expr::portTokens
        Expr::portTokens(*getImpl()) >= m;
    }

    PortTokensGuard getConsumableTokens()
      { return Expr::portTokens(*getImpl()); }

    // reflect operator () to channel interface
    CommAndPortTokensGuard operator ()(size_t n, size_t m)
      { return this->communicate(n,m); }
    CommAndPortTokensGuard operator ()(size_t n)
      { return this->communicate(n,n); }

    // Provide [] access operator for port.
    const return_type operator[](size_t n) const {
#ifdef SYSTEMOC_PORT_ACCESS_COUNTER
      (*getImpl())->incrementAccessCount();
#endif // SYSTEMOC_PORT_ACCESS_COUNTER
      return (*(getImpl()->get_chanaccess()))[n];
    }

    // Provide getValueAt method for port. The methods returms an expression
    // corresponding to the token value in the fifo at offset n for usage in
    // transition guards
    typename smoc::Expr::Token<IFACE>::type getValueAt(size_t n)
      { return smoc::Expr::token<IFACE>(*getImpl(),n); }

    // Provide tokenIsValid method for port. The methods returns true if the
    // token on offset i is defined or false otherwise.
    bool tokenIsValid(size_t i=0) const
      { return getImpl()->get_chanaccess()->tokenIsValid(i); }
  };
protected:
  // constructor
  PortInBaseIf() {}
 
#ifdef SYSTEMOC_ENABLE_VPC
  virtual void        commitRead(size_t consume,
    VpcInterface vpcIf) = 0;
#else //!defined(SYSTEMOC_ENABLE_VPC)
  virtual void        commitRead(size_t consume) = 0;
#endif //!defined(SYSTEMOC_ENABLE_VPC)

  virtual smoc_event &dataAvailableEvent(size_t n) = 0;

  smoc_event &blockEvent(size_t n)
    { return this->dataAvailableEvent(n); }  
  size_t availableCount() const
    { return this->numAvailable(); }
#ifdef SYSTEMOC_ENABLE_VPC
  void commExec(
      size_t n,
      VpcInterface vpcIf)
    {
      vpcIf.setPortIf(this); // TODO (ms): move to base class?
      vpcIf.startVpcRead(n);
      return this->commitRead(n, vpcIf);
    }
#else //!defined(SYSTEMOC_ENABLE_VPC)
  void commExec(size_t n)
    { return this->commitRead(n); }
#endif //!defined(SYSTEMOC_ENABLE_VPC)

  /// @brief More tokens available
  virtual void moreData(size_t n) {}
  /// @brief Less tokens available
  virtual void lessData(size_t n) {}
  /// @brief Reset
  virtual void reset() {}

#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
  virtual void traceCommSetup(size_t req) {};
#endif // SYSTEMOC_ENABLE_DATAFLOW_TRACE

public:
  virtual size_t      inTokenId() const = 0;
  virtual size_t      numAvailable() const = 0;
  virtual std::string getChannelName() const = 0;

  virtual ~PortInBaseIf() {}
};

class PortOutBaseIf: public PortBaseIf {
  typedef PortOutBaseIf this_type;

  friend class ::smoc_graph_synth;
  friend class ::smoc_multicast_sr_signal_chan_base;
  friend class ::smoc_multireader_fifo_chan_base;
  friend class ::smoc_signal_chan_base;
  template<class,class> friend class ::smoc_multiplex_fifo_chan;
  template<class,class> friend class ::smoc_chan_adapter;

  template<class>       friend class ChanAdapterMid;

  // Friends needed for guard evaluation
  template <class E> friend class Expr::Sensitivity;
  template <class E> friend class Expr::CommExec;
#if defined(SYSTEMOC_ENABLE_DEBUG)
  template <class E> friend class Expr::CommReset;
  template <class E> friend class Expr::CommSetup;
#endif
  template <class E> friend class Expr::Value;
public:
  template <class PORT, class IFACE>
  class PortMixin {
  private:
    PORT       *getImpl()
      { return static_cast<PORT       *>(this); }
    PORT const *getImpl() const
      { return static_cast<PORT const *>(this); }
  public:
    typedef this_type                         chan_base_type;
    typedef IFACE                             iface_type;
    typedef typename iface_type::access_type  access_type;
    typedef typename iface_type::data_type    data_type;
    typedef typename access_type::return_type return_type;

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
        Expr::comm(*getImpl(),
                   Expr::DLiteral<size_t>(n),
                   Expr::DLiteral<size_t>(m))
        && //FIXME: Expr::comm knows n and m -> should remove Expr::portTokens
        Expr::portTokens(*getImpl()) >= m;
    }

    PortTokensGuard getFreeSpace()
      { return Expr::portTokens(*getImpl()); }

    // reflect operator () to channel interface
    CommAndPortTokensGuard operator ()(size_t n, size_t m)
      { return this->communicate(n,m); }
    CommAndPortTokensGuard operator ()(size_t n)
      { return this->communicate(n,n); }

    // Provide [] access operator for port.
    return_type operator[](size_t n)  {
#ifdef SYSTEMOC_PORT_ACCESS_COUNTER
      (*getImpl())->incrementAccessCount();
#endif // SYSTEMOC_PORT_ACCESS_COUNTER
      return (*(getImpl()->get_chanaccess()))[n];
    }
  };
protected:
  // constructor
  PortOutBaseIf() {}

#ifdef SYSTEMOC_ENABLE_VPC
  virtual void        commitWrite(size_t produce,
    VpcInterface vpcIf) = 0;
#else //!defined(SYSTEMOC_ENABLE_VPC)
  virtual void        commitWrite(size_t produce) = 0;
#endif //!defined(SYSTEMOC_ENABLE_VPC)

  virtual smoc_event &spaceAvailableEvent(size_t n) = 0;

  smoc_event &blockEvent(size_t n)
    { return this->spaceAvailableEvent(n); }  
  size_t availableCount() const
    { return this->numFree(); }
#ifdef SYSTEMOC_ENABLE_VPC
  void commExec(
      size_t n,
      VpcInterface vpcIf)
    { vpcIf.setPortIf(this); return this->commitWrite(n, vpcIf); }
#else //!defined(SYSTEMOC_ENABLE_VPC)
  void commExec(size_t n)
    { return this->commitWrite(n); }
#endif //!defined(SYSTEMOC_ENABLE_VPC)

  /// @brief More free space available
  virtual void moreSpace(size_t n) {}
  /// @brief Less free space available
  virtual void lessSpace(size_t n) {}
  /// @brief Reset
  virtual void reset() {}

#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
  virtual void traceCommSetup(size_t req) {};
#endif // SYSTEMOC_ENABLE_DATAFLOW_TRACE

public:
  virtual size_t      outTokenId() const = 0;
  virtual size_t      numFree() const = 0;
  virtual std::string getChannelName() const = 0;

  virtual ~PortOutBaseIf() {}
};

} } // namespace smoc::Detail

#endif //_INCLUDED_SMOC_DETAIL_PORTIOBASEIF_HPP
