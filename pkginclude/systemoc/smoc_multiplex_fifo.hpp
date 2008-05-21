//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2008 Hardware-Software-CoDesign, University of
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

#ifndef _INCLUDED_SMOC_MULTIPLEX_FIFO_HPP
#define _INCLUDED_SMOC_MULTIPLEX_FIFO_HPP

#include <CoSupport/commondefs.h>

#include <systemoc/smoc_config.h>

#include "smoc_chan_if.hpp"
#include "smoc_storage.hpp"
#include "smoc_chan_adapter.hpp"
#include "detail/smoc_latency_queues.hpp"
#include "detail/smoc_ring_access.hpp"
#include "detail/EventMapManager.hpp"
#include "detail/Queue3Ptr.hpp"

#include <systemc.h>
#include <vector>
#include <queue>
#include <map>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include "hscd_tdsim_TraceLog.hpp"

#ifdef SYSTEMOC_ENABLE_VPC
# include <systemcvpc/hscd_vpc_Director.h>
#endif //SYSTEMOC_ENABLE_VPC

class smoc_multiplex_vfifo_kind;

class smoc_multiplex_fifo_kind
//: public smoc_nonconflicting_chan,
: public boost::noncopyable,
#ifdef SYSTEMOC_ENABLE_VPC
  public Detail::Queue3Ptr
#else
  public Detail::Queue2Ptr
#endif // SYSTEMOC_ENABLE_VPC
{
#ifdef SYSTEMOC_ENABLE_VPC
  friend class Detail::LatencyQueue<smoc_multiplex_fifo_kind>::VisibleQueue;
  friend class Detail::LatencyQueue<smoc_multiplex_fifo_kind>::RequestQueue;
#endif // SYSTEMOC_ENABLE_VPC
  friend class smoc_multiplex_vfifo_kind;
public:
  typedef smoc_multiplex_fifo_kind  this_type;
  
  typedef size_t FifoId;
private:
  typedef std::list<FifoId> FifoSequence;
//private:
protected:
#ifdef SYSTEMOC_ENABLE_VPC
  SystemC_VPC::FastLink *vpcLink; // FIXME: patch this in!!!
#endif //SYSTEMOC_ENABLE_VPC
public://FIXME
  FifoId                                        fifoIdCount;  // For virtual fifo enumeration
  std::map<FifoId, smoc_multiplex_vfifo_kind *> vFifos;

  void registerVFifo(smoc_multiplex_vfifo_kind *vfifo);
  void deregisterVFifo(smoc_multiplex_vfifo_kind *vfifo);

  FifoSequence            fifoSequence;
  FifoSequence            fifoSequenceOOO;
  const size_t            fifoOutOfOrder; // == 0 => no out of order access only one element visible
  Detail::EventMapManager emmFree;
#ifdef SYSTEMOC_ENABLE_VPC
  Detail::LatencyQueue<smoc_multiplex_fifo_kind> latencyQueue;
#endif

  smoc_event &spaceAvailableEvent(size_t n)
    { return emmFree.getEvent(freeCount(), n); }

  void consume(FifoId from, size_t n);

#ifdef SYSTEMOC_ENABLE_VPC
  void produce(FifoId to, size_t n, const smoc_ref_event_p &le)
#else
  void produce(FifoId to, size_t n)
#endif
  {
    wpp(n);
    for (size_t j = n; j > 0; --j)
      fifoSequence.push_back(to);
    emmFree.decreasedCount(freeCount());
#ifdef SYSTEMOC_ENABLE_VPC
    latencyQueue.addEntry(n, le); // Delayed call of latencyExpired(n);
#else
    latencyExpired(n);
#endif
  }

  void latencyExpired(size_t n);

public:
  smoc_multiplex_fifo_kind(const char *name, size_t n, size_t m);
};

typedef boost::shared_ptr<smoc_multiplex_fifo_kind>  p_smoc_multiplex_fifo_kind;

class smoc_multiplex_vfifo_kind
: public smoc_nonconflicting_chan,
  public boost::noncopyable,
  // due to out of order access we always need a visible area management
  public Detail::Queue3Ptr 
{
  friend class smoc_multiplex_fifo_kind;
public:
  typedef smoc_multiplex_vfifo_kind  this_type;

  typedef size_t FifoId;

  class chan_init {
    friend class smoc_multiplex_vfifo_kind;
  private:
    const char                 *name;
    p_smoc_multiplex_fifo_kind  pSharedFifoMem;
    FifoId                      fifoId;
    size_t                      m;
  protected:
    chan_init(const char *name, const p_smoc_multiplex_fifo_kind &pSharedFifoMem, size_t m)
      : name(name), pSharedFifoMem(pSharedFifoMem),
        fifoId(pSharedFifoMem->fifoIdCount++), m(m) {}
  };
private:
  Detail::EventMapManager emmAvailable;
  FifoId fifoId;
  p_smoc_multiplex_fifo_kind pSharedFifoMem;
protected:
  // constructors
  smoc_multiplex_vfifo_kind(const chan_init &i);

  ~smoc_multiplex_vfifo_kind();

  smoc_event &dataAvailableEvent(size_t n)
    { return emmAvailable.getEvent(visibleCount(), n); }

  smoc_event &spaceAvailableEvent(size_t n)
    { return pSharedFifoMem->spaceAvailableEvent(n); }

  size_t tokenId; ///< The tokenId of the next commit token

  size_t inTokenId() const
    { return tokenId - usedCount(); }
  size_t outTokenId() const
    { return tokenId; }

#ifdef SYSTEMOC_ENABLE_VPC
  void commitRead(size_t consume, const smoc_ref_event_p &le)
#else
  void commitRead(size_t consume)
#endif
  {
#ifdef SYSTEMOC_TRACE
    TraceLog.traceCommExecIn(this, consume);
#endif
    rpp(consume);
    emmAvailable.decreasedCount(visibleCount());
    pSharedFifoMem->consume(fifoId, consume);
  }

#ifdef SYSTEMOC_ENABLE_VPC
  void commitWrite(size_t produce, const smoc_ref_event_p &le)
#else
  void commitWrite(size_t produce)
#endif
  {
#ifdef SYSTEMOC_TRACE
    TraceLog.traceCommExecOut(this, produce);
#endif
    tokenId += produce;
    wpp(produce);
    // This will do a callback to latencyExpired(produce) at the appropriate time
#ifdef SYSTEMOC_ENABLE_VPC
    pSharedFifoMem->produce(fifoId, produce, le); 
#else
    pSharedFifoMem->produce(fifoId, produce); 
#endif
  }

  void latencyExpired(size_t n) {
    vpp(n);
    emmAvailable.increasedCount(visibleCount());
  }

  // bounce functions
  size_t numAvailable() const
    { return visibleCount(); }
  size_t numFree() const
    { return freeCount(); }

  void channelAttributes(smoc_modes::PGWriter &pgw) const {
    pgw << "<attribute type=\"size\" value=\"" << depthCount() << "\"/>" << std::endl;
  }

  virtual
  void channelContents(smoc_modes::PGWriter &pgw) const = 0;

public:
  virtual const char* kind() const
    { return "smoc_multiplex_vfifo_kind"; }
};

inline
void smoc_multiplex_fifo_kind::registerVFifo(smoc_multiplex_vfifo_kind *vfifo) {
  vFifos[vfifo->fifoId] = vfifo;
}

inline
void smoc_multiplex_fifo_kind::deregisterVFifo(smoc_multiplex_vfifo_kind *vfifo) {
  vFifos.erase(vfifo->fifoId);
  for (FifoSequence::iterator iter = fifoSequence.begin();
       iter !=  fifoSequence.end();
       ) {
    if (*iter == vfifo->fifoId) {
      iter = fifoSequence.erase(iter);
    } else {
      ++iter;
    }
  }
}


template <typename T>
class smoc_multiplex_vfifo_storage
: public smoc_chan_if<
    T,
    smoc_channel_access,
    smoc_channel_access>,
  public smoc_multiplex_vfifo_kind {
public:
  typedef T                                   data_type;
  typedef smoc_multiplex_vfifo_storage<data_type>        this_type;
  typedef typename this_type::access_out_type ring_out_type;
  typedef typename this_type::access_in_type  ring_in_type;
  typedef smoc_storage<data_type>             storage_type;
  typedef smoc_ring_access<
    storage_type,
    typename ring_in_type::return_type>       ring_access_in_type;
  typedef smoc_ring_access<
    storage_type,
    typename ring_out_type::return_type>      ring_access_out_type;

  class chan_init
    : public smoc_multiplex_vfifo_kind::chan_init {
    friend class smoc_multiplex_vfifo_storage<T>;
  private:
    std::vector<T>  marking;
  protected:
    typedef const T add_param_ty;
  public:
    void add( add_param_ty x ) {
      marking.push_back(x);
    }
  protected:
    chan_init(const char *name, const p_smoc_multiplex_fifo_kind &pSharedFifoMem, size_t m)
      : smoc_multiplex_vfifo_kind::chan_init(name, pSharedFifoMem, m) {}
  };
private:
  storage_type *storage;
protected:
  smoc_multiplex_vfifo_storage( const chan_init &i )
    : smoc_multiplex_vfifo_kind(i),
      storage(new storage_type[this->fSize()])
  {
    assert(this->depthCount() >= i.marking.size());
    for(size_t j = 0; j < i.marking.size(); ++j) {
      storage[j].put(i.marking[j]);
    }
    wpp(i.marking.size());
    // FIXME: What about vpp does smoc_multiplex_fifo_kind trigger this
    // for initial tokens?
  }
  
  const char *name() const
    { return smoc_multiplex_vfifo_kind::name(); }

  ring_in_type *getReadChannelAccess() {
    return new ring_access_in_type
      (storage, this->fSize(), &this->rIndex());
  }
  ring_out_type *getWriteChannelAccess() {
    return new ring_access_out_type
      (storage, this->fSize(), &this->wIndex());
  }

  void channelContents(smoc_modes::PGWriter &pgw) const {
    pgw << "<fifo tokenType=\"" << typeid(data_type).name() << "\">" << std::endl;
    {
      //*************************INITIAL TOKENS, ETC...***************************
      pgw.indentUp();
      for ( size_t n = 0; n < this->visibleCount(); ++n )
        pgw << "<token value=\"" << storage[n].get() << "\"/>" << std::endl;
      pgw.indentDown();
    }
    pgw << "</fifo>" << std::endl;
  }

  ~smoc_multiplex_vfifo_storage() { delete[] storage; }
};

template <>
class smoc_multiplex_vfifo_storage<void>
: public smoc_chan_if<
    void,
    smoc_channel_access,
    smoc_channel_access>,
  public smoc_multiplex_vfifo_kind {
public:
  typedef void                          data_type;
  typedef smoc_multiplex_vfifo_storage<data_type>  this_type;
  typedef this_type::access_out_type    ring_out_type;
  typedef this_type::access_in_type     ring_in_type;
  typedef smoc_ring_access<void,void>   ring_access_in_type;
  typedef smoc_ring_access<void,void>   ring_access_out_type;

  class chan_init
    : public smoc_multiplex_vfifo_kind::chan_init {
    friend class smoc_multiplex_vfifo_storage<void>;
  private:
    size_t          marking;
  protected:
    typedef size_t  add_param_ty;
  public:
    void add( add_param_ty x ) {
      marking += x;
    }
  protected:
    chan_init(const char *name, const p_smoc_multiplex_fifo_kind &pSharedFifoMem, size_t m)
      : smoc_multiplex_vfifo_kind::chan_init(name, pSharedFifoMem, m),
        marking(0) {}
  };
protected:
  smoc_multiplex_vfifo_storage( const chan_init &i ) :
    smoc_multiplex_vfifo_kind(i) {
    wpp(i.marking);
    // FIXME: What about vpp does smoc_multiplex_fifo_kind trigger this
    // for initial tokens?
  }

  const char *name() const
    { return smoc_multiplex_vfifo_kind::name(); }

  ring_in_type  *getReadChannelAccess()
    { return new ring_access_in_type(); }
  ring_out_type *getWriteChannelAccess()
    { return new ring_access_out_type(); }

  void channelContents(smoc_modes::PGWriter &pgw) const {
    pgw << "<fifo tokenType=\"" << typeid(data_type).name() << "\">" << std::endl;
    {
      //*************************INITIAL TOKENS, ETC...***************************
      pgw.indentUp();
      for ( size_t n = 0; n < this->visibleCount(); ++n )
        pgw << "<token value=\"bot\"/>" << std::endl;
      pgw.indentDown();
    }
    pgw << "</fifo>" << std::endl;
  }
};

template <typename T>
class smoc_multiplex_vfifo_type
  : public smoc_multiplex_vfifo_storage<T> {
public:
  typedef T                  data_type;
  typedef smoc_multiplex_vfifo_type<data_type>            this_type;
  
  typedef typename smoc_storage_in<data_type>::storage_type   storage_in_type;
  typedef typename smoc_storage_in<data_type>::return_type    return_in_type;
  
  typedef typename smoc_storage_out<data_type>::storage_type  storage_out_type;
  typedef typename smoc_storage_out<data_type>::return_type   return_out_type;
public:
  // constructors
  smoc_multiplex_vfifo_type( const typename smoc_multiplex_vfifo_storage<T>::chan_init &i )
    : smoc_multiplex_vfifo_storage<T>(i) {}

  template<class IFace,class Init>
  void connect(sc_port<IFace> &port, const Init&) {

    using namespace SysteMoC::Detail;

    // we can provide smoc_chan_out_if and smoc_chan_in_if
    // interfaces
    typedef typename smoc_port_out<T>::iface_type IFaceImplOut;
    typedef typename smoc_port_in<T>::iface_type  IFaceImplIn;

    // corresponding adapters
    typedef smoc_chan_adapter<IFaceImplOut,IFace> AdapterOut;
    typedef smoc_chan_adapter<IFaceImplIn,IFace>  AdapterIn;

    typedef
      // 1st possible adapter
      typename Select<
        AdapterOut::isAdapter,
        Alloc<IFaceImplOut, AdapterOut>,
      // 2nd possible adapter
      typename Select<
        AdapterIn::isAdapter,
        Alloc<IFaceImplIn, AdapterIn>,
      // otherwise -> error
      void
    >::result_type
    >::result_type Op;

    port(Op::apply(*this));
  }

  template<class Init>
  void connect(smoc_port_out<T> &outPort, const Init&)
  { outPort(*this); }

  template<class Init>
  void connect(smoc_port_in<T> &inPort, const Init&)
  { inPort(*this); }
private:
    void reset(){};
};

template <typename T>
class smoc_multiplex_fifo {
private:
  typedef smoc_multiplex_fifo<T>        this_type;
private:
  typedef size_t FifoId;
public:

  class VirtFifo: public smoc_multiplex_vfifo_type<T>::chan_init {
    friend class smoc_multiplex_fifo<T>;
  public:
    typedef T                             data_type;
    typedef smoc_multiplex_vfifo_type<T>  chan_type;
  private:
    VirtFifo(const p_smoc_multiplex_fifo_kind &pSharedFifoMem, size_t m)
      : smoc_multiplex_vfifo_type<T>::chan_init("", pSharedFifoMem, m) {}
  public:
    this_type &operator <<(const T &x) {
      add(x);
      pSharedFifoMem->produce(this->fifoId, 1);
      return *this;
    }
  };

  friend class VirtFifo;
private:
  size_t  n;  // size of the shared fifo memory
  size_t  m;  // out of order access, zero is no out of order

  p_smoc_multiplex_fifo_kind pSharedFifoMem;

public:
  /// @PARAM n size of the shared fifo memory
  /// @PARAM m out of order access, zero is no out of order
  smoc_multiplex_fifo(size_t n = 1, size_t m = 0)
    : n(n), m(m), pSharedFifoMem(new smoc_multiplex_fifo_kind(NULL, n, m)) {}
  smoc_multiplex_fifo(const char *name, size_t n = 1, size_t m = 0)
    : n(n), m(m), pSharedFifoMem(new smoc_multiplex_fifo_kind(name, n, m)) {}

  VirtFifo getVirtFifo()
    { return VirtFifo(pSharedFifoMem, m); }
};

#endif // _INCLUDED_SMOC_MULTIPLEX_FIFO_HPP