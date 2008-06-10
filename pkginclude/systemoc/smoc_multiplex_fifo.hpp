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

#include "detail/smoc_root_chan.hpp"
#include "smoc_chan_if.hpp"
#include "smoc_storage.hpp"
#include "smoc_chan_adapter.hpp"
#include "detail/smoc_latency_queues.hpp"
#include "detail/smoc_ring_access.hpp"
#include "detail/EventMapManager.hpp"
#include "detail/QueueRVWPtr.hpp"
#include "detail/QueueFRVWPtr.hpp"

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

class smoc_multiplex_vfifo_chan_base;

class smoc_multiplex_fifo_chan
: private boost::noncopyable,
  public smoc_root_chan,
#ifdef SYSTEMOC_ENABLE_VPC
  public Detail::LatencyQueue::ILatencyExpired,
  public Detail::DIIQueue::IDIIExpired,
  public Detail::QueueFRVWPtr
#else
  public Detail::QueueRWPtr
#endif // SYSTEMOC_ENABLE_VPC
{
public:
  friend class smoc_multiplex_vfifo_chan_base;
  friend class smoc_multiplex_vfifo_outlet_base;
  friend class smoc_multiplex_vfifo_entry_base;
  
  typedef smoc_multiplex_fifo_chan this_type;
  typedef size_t FifoId;
  typedef std::list<FifoId> FifoSequence;
  typedef std::map<FifoId, smoc_multiplex_vfifo_chan_base *> FifoMap;

  smoc_multiplex_fifo_chan(const std::string& name, size_t n, size_t m);

protected:
  void registerVFifo(smoc_multiplex_vfifo_chan_base *vfifo);
  void deregisterVFifo(smoc_multiplex_vfifo_chan_base *vfifo);

  smoc_event &spaceAvailableEvent(size_t n)
    { return emmFree.getEvent(freeCount(), n); }

#ifdef SYSTEMOC_ENABLE_VPC
  void consume(FifoId from, size_t n, const smoc_ref_event_p &diiEvent);
#else
  void consume(FifoId from, size_t n);
#endif

#ifdef SYSTEMOC_ENABLE_VPC
  void produce(FifoId to, size_t n, const smoc_ref_event_p &latEvent);
#else
  void produce(FifoId to, size_t n);
#endif

  /// @brief Detail::LatencyQueue::ILatencyExpired
  void latencyExpired(size_t n);

  void diiExpired(size_t n);

  /// @brief See smoc_root_chan
  void assemble(smoc_modes::PGWriter &pgw) const
    {}
  
  /// @brief See smoc_root_chan
  void channelContents(smoc_modes::PGWriter &pgw) const
    { assert(0); }
  
  /// @brief See smoc_root_chan
  virtual void channelAttributes(smoc_modes::PGWriter &pgw) const
    { assert(0); }

  /// @brief See smoc_port_registry
  smoc_chan_in_base_if* createOutlet()
    { assert(!"Use virtual FIFOs!"); }

  /// @brief See smoc_port_registry
  smoc_chan_out_base_if* createEntry()
    { assert(!"Use virtual FIFOs!"); }

public:
  ///gcc3.4.6: Inner classes are not friends when outer class is declared as friend
  FifoId getNewFifoId()
    { return fifoIdCount++; }

private:
  FifoId        fifoIdCount;  // For virtual fifo enumeration
  FifoMap       vFifos;
  FifoSequence  fifoSequence;
  FifoSequence  fifoSequenceOOO;
  const size_t  fifoOutOfOrder; // == 0 => no out of order access only one element visible

  Detail::EventMapManager emmFree;
#ifdef SYSTEMOC_ENABLE_VPC
  Detail::LatencyQueue  latencyQueue;
  Detail::DIIQueue      diiQueue;
#endif
};

typedef boost::shared_ptr<smoc_multiplex_fifo_chan>  p_smoc_multiplex_fifo_chan;

class smoc_multiplex_vfifo_chan_base
: private boost::noncopyable,
  public smoc_nonconflicting_chan,
  // due to out of order access we always need a visible area management
  public Detail::QueueRVWPtr 
{
public:
  friend class smoc_multiplex_fifo_chan;
  friend class smoc_multiplex_vfifo_outlet_base;
  friend class smoc_multiplex_vfifo_entry_base;

  typedef smoc_multiplex_vfifo_chan_base  this_type;
  typedef size_t FifoId;

  /// @brief Channel initializer
  class chan_init {
  public:
    friend class smoc_multiplex_vfifo_chan_base;
  protected:
    chan_init(const std::string& name, const p_smoc_multiplex_fifo_chan &pSharedFifoMem, size_t m)
      : name(name), pSharedFifoMem(pSharedFifoMem),
        fifoId(pSharedFifoMem->getNewFifoId()), m(m)
    {}
  private:
    std::string                 name;
    p_smoc_multiplex_fifo_chan  pSharedFifoMem;
    FifoId                      fifoId;
    size_t                      m;
  };

private:
  Detail::EventMapManager emmAvailable;
  FifoId fifoId;
  p_smoc_multiplex_fifo_chan pSharedFifoMem;
protected:
  // constructors
  smoc_multiplex_vfifo_chan_base(const chan_init &i);

  ~smoc_multiplex_vfifo_chan_base();

  /// @brief The tokenId of the next commit token
  size_t tokenId;

  void latencyExpired(size_t n) {
    vpp(n);
    emmAvailable.increasedCount(visibleCount());
  }

  /// @brief See smoc_root_chan
  void channelAttributes(smoc_modes::PGWriter &pgw) const {
    pgw << "<attribute type=\"size\" value=\"" << depthCount() << "\"/>" << std::endl;
  }
};

class smoc_multiplex_vfifo_outlet_base
: public virtual smoc_chan_in_base_if
{
public:
protected:
  /// @brief Constructor
  smoc_multiplex_vfifo_outlet_base(smoc_multiplex_vfifo_chan_base &chan)
    : chan(chan)
  {}

  /// @brief See smoc_chan_in_base_if
#ifdef SYSTEMOC_ENABLE_VPC
  void commitRead(size_t consume, const smoc_ref_event_p &diiEvent)
#else
  void commitRead(size_t consume)
#endif
  {
#ifdef SYSTEMOC_TRACE
    TraceLog.traceCommExecIn(&chan, consume);
#endif
    chan.rpp(consume);
    chan.emmAvailable.decreasedCount(chan.visibleCount());
#ifdef SYSTEMOC_ENABLE_VPC
    chan.pSharedFifoMem->consume(chan.fifoId, consume, diiEvent);
#else
    chan.pSharedFifoMem->consume(chan.fifoId, consume);
#endif
  }

  /// @brief See smoc_chan_in_base_if
  smoc_event &dataAvailableEvent(size_t n)
    { return chan.emmAvailable.getEvent(chan.visibleCount(), n); }

  /// @brief See smoc_chan_in_base_if
  size_t numAvailable() const
    { return chan.visibleCount(); }

  /// @brief See smoc_chan_in_base_if
  size_t inTokenId() const
    { return chan.tokenId - chan.usedCount(); }

private:
  /// @brief The channel base implementation
  smoc_multiplex_vfifo_chan_base &chan;
};

class smoc_multiplex_vfifo_entry_base
: public virtual smoc_chan_out_base_if
{
public:
protected:
  /// @brief Constructor
  smoc_multiplex_vfifo_entry_base(smoc_multiplex_vfifo_chan_base &chan)
    : chan(chan)
  {}

  /// @brief See smoc_chan_out_base_if
#ifdef SYSTEMOC_ENABLE_VPC
  void commitWrite(size_t produce, const smoc_ref_event_p &latEvent)
#else
  void commitWrite(size_t produce)
#endif
  {
#ifdef SYSTEMOC_TRACE
    TraceLog.traceCommExecOut(&chan, produce);
#endif
    chan.tokenId += produce;
    chan.wpp(produce);
    // This will do a callback to latencyExpired(produce) at the appropriate time
#ifdef SYSTEMOC_ENABLE_VPC
    chan.pSharedFifoMem->produce(chan.fifoId, produce, latEvent); 
#else
    chan.pSharedFifoMem->produce(chan.fifoId, produce); 
#endif
  }

  /// @brief See smoc_chan_out_base_if
  smoc_event &spaceAvailableEvent(size_t n)
    { return chan.pSharedFifoMem->spaceAvailableEvent(n); }

  /// @brief See smoc_chan_out_base_if
  size_t numFree() const
    { return chan.freeCount(); }

  /// @brief See smoc_chan_out_base_if
  size_t outTokenId() const
    { return chan.tokenId; }

private:
  /// @brief The channel base implementation
  smoc_multiplex_vfifo_chan_base &chan;
};

template<class> class smoc_multiplex_vfifo_chan;

template<class T>
class smoc_multiplex_vfifo_outlet
: public smoc_multiplex_vfifo_outlet_base,
  public smoc_chan_in_if<T,smoc_channel_access>
{
public:
  typedef smoc_multiplex_vfifo_outlet<T> this_type;
  typedef typename this_type::access_type access_type;
  typedef smoc_chan_in_if<T,smoc_channel_access> iface_type;

  /// @brief Constructor
  smoc_multiplex_vfifo_outlet(smoc_multiplex_vfifo_chan<T>& chan)
    : smoc_multiplex_vfifo_outlet_base(chan),
      chan(chan)
  {}

protected:

  /// @brief See smoc_chan_in_if
  access_type *getReadChannelAccess()
    { return chan.getReadChannelAccess(); }

private:
  /// @brief The channel implementation
  smoc_multiplex_vfifo_chan<T>& chan;
};

template<class T>
class smoc_multiplex_vfifo_entry
: public smoc_multiplex_vfifo_entry_base,
  public smoc_chan_out_if<T,smoc_channel_access>
{
public:
  typedef smoc_multiplex_vfifo_entry<T> this_type;
  typedef typename this_type::access_type access_type;
  typedef smoc_chan_out_if<T,smoc_channel_access> iface_type;

  /// @brief Constructor
  smoc_multiplex_vfifo_entry(smoc_multiplex_vfifo_chan<T>& chan)
    : smoc_multiplex_vfifo_entry_base(chan),
      chan(chan)
  {}

protected:

  /// @brief See smoc_chan_out_if
  access_type *getWriteChannelAccess()
    { return chan.getWriteChannelAccess(); }

private:
  /// @brief The channel implementation
  smoc_multiplex_vfifo_chan<T>& chan;
};


template <typename T>
class smoc_multiplex_vfifo_storage
: public smoc_multiplex_vfifo_chan_base
{
public:
  typedef T                                       data_type;
  typedef smoc_multiplex_vfifo_outlet<data_type>   outlet_type;
  typedef smoc_multiplex_vfifo_entry<data_type>  entry_type;
  typedef smoc_storage<data_type>                 storage_type;

  typedef typename outlet_type::access_type  access_in_type;
  typedef typename entry_type::access_type access_out_type;

  typedef smoc_ring_access<
    storage_type,
    typename access_in_type::return_type> access_in_type_impl;
  typedef smoc_ring_access<
    storage_type,
    typename access_out_type::return_type> access_out_type_impl;

  friend class smoc_multiplex_vfifo_outlet<data_type>;
  friend class smoc_multiplex_vfifo_entry<data_type>;

  class chan_init
  : public smoc_multiplex_vfifo_chan_base::chan_init
  {
  public:
    friend class smoc_multiplex_vfifo_storage<T>;
    typedef const T add_param_ty;
    
    void add(const add_param_ty &x)
      { marking.push_back(x); }
  protected:
    chan_init(const std::string& name, const p_smoc_multiplex_fifo_chan &pSharedFifoMem, size_t m)
      : smoc_multiplex_vfifo_chan_base::chan_init(name, pSharedFifoMem, m)
    {}
  private:
    std::vector<T> marking;
  };

protected:
  smoc_multiplex_vfifo_storage( const chan_init &i )
    : smoc_multiplex_vfifo_chan_base(i),
      storage(new storage_type[this->fSize()])
  {
    assert(this->depthCount() >= i.marking.size());
    for(size_t j = 0; j < i.marking.size(); ++j) {
      storage[j].put(i.marking[j]);
    }
    wpp(i.marking.size());
    // FIXME: What about vpp does smoc_multiplex_fifo_chan trigger this
    // for initial tokens?
  }
  
  ~smoc_multiplex_vfifo_storage()
    { delete[] storage; }

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

  access_in_type *getReadChannelAccess() {
    return new access_in_type_impl(
        storage, this->fSize(), &this->rIndex());
  }
  
  access_out_type *getWriteChannelAccess() {
    return new access_out_type_impl(
        storage, this->fSize(), &this->wIndex());
  }

private:
  storage_type *storage;
};

template<>
class smoc_multiplex_vfifo_storage<void>
: public smoc_multiplex_vfifo_chan_base
{
public:
  typedef void                                    data_type;
  typedef smoc_multiplex_vfifo_outlet<data_type>   outlet_type;
  typedef smoc_multiplex_vfifo_entry<data_type>  entry_type;
  typedef smoc_storage<data_type>                 storage_type;

  typedef outlet_type::access_type  access_in_type;
  typedef entry_type::access_type access_out_type;

  typedef smoc_ring_access<void,void> access_in_type_impl;
  typedef smoc_ring_access<void,void> access_out_type_impl;

  friend class smoc_multiplex_vfifo_outlet<data_type>;
  friend class smoc_multiplex_vfifo_entry<data_type>;

  /// @brief Channel initializer
  class chan_init
  : public smoc_multiplex_vfifo_chan_base::chan_init
  {
  public:
    friend class smoc_multiplex_vfifo_storage;
    typedef size_t add_param_ty;

    void add(const add_param_ty &t) {
      marking += t;
    }
  protected:
    chan_init(const std::string& name, const p_smoc_multiplex_fifo_chan &pSharedFifoMem, size_t m)
      : smoc_multiplex_vfifo_chan_base::chan_init(name, pSharedFifoMem, m),
        marking(0)
    {}
  private:
    size_t marking;
  };

protected:

  /// @brief Constructor
  smoc_multiplex_vfifo_storage(const chan_init &i)
    : smoc_multiplex_vfifo_chan_base(i)
  {
    assert(this->depthCount() >= i.marking);
    wpp(i.marking);
    // FIXME: What about vpp does smoc_multiplex_fifo_chan trigger this
    // for initial tokens?
  }

  /// @brief See smoc_root_chan
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

  access_in_type *getReadChannelAccess()
    { return new access_in_type_impl(); }

  access_out_type *getWriteChannelAccess()
    { return new access_out_type_impl(); }
};

template<class T>
class smoc_multiplex_vfifo_chan
: public smoc_multiplex_vfifo_storage<T>
{
public:
  typedef T                                       data_type;
  typedef smoc_multiplex_vfifo_chan<data_type>    this_type;
  typedef smoc_multiplex_vfifo_entry<data_type>   entry_type;
  typedef smoc_multiplex_vfifo_outlet<data_type>  outlet_type;
  
  typedef typename entry_type::iface_type   entry_iface_type;
  typedef typename outlet_type::iface_type  outlet_iface_type;

  /// @brief Channel initializer
  typedef typename smoc_multiplex_vfifo_storage<T>::chan_init chan_init;

  /// @brief Constructor
  smoc_multiplex_vfifo_chan(const chan_init &i)
    : smoc_multiplex_vfifo_storage<T>(i)
  {}

  /// @brief Nicer compile time error
  struct No_Channel_Adapter_Found__Please_Use_Other_Interface {};
  
  /// @brief Connect sc_port
  template<class IFace,class Init>
  void connect(sc_port<IFace>& p, const Init&) {
  
    using namespace SysteMoC::Detail;

    // available adapters
    typedef smoc_chan_adapter<entry_iface_type,IFace>   EntryAdapter;
    typedef smoc_chan_adapter<outlet_iface_type,IFace>  OutletAdapter;

    // try to get adapter (utilize Tags for simpler implementation)
    typedef
      typename Select<
        EntryAdapter::isAdapter,
        std::pair<EntryAdapter,smoc_port_registry::EntryTag>,
      typename Select<
        OutletAdapter::isAdapter,
        std::pair<OutletAdapter,smoc_port_registry::OutletTag>,
      No_Channel_Adapter_Found__Please_Use_Other_Interface
      >::result_type
      >::result_type P;

    // corresponding types
    typedef typename P::first_type Adapter;
    typedef typename P::second_type Tag;

    typename Adapter::iface_impl_type* iface =
      dynamic_cast<typename Adapter::iface_impl_type*>(
          smoc_port_registry::getIF<Tag>(&p));
    assert(iface); p(*(new Adapter(*iface)));
  }
  
  /// @brief Connect smoc_port_out
  template<class Init>
  void connect(smoc_port_out<data_type>& p, const Init&) {
    entry_type* e =
      dynamic_cast<entry_type*>(getEntry(&p));
    assert(e); p(*e);
  }

  /// @brief Connect smoc_port_in
  template<class Init>
  void connect(smoc_port_in<data_type>& p, const Init&) {
    outlet_type* o =
      dynamic_cast<outlet_type*>(getOutlet(&p));
    assert(o); p(*o);
  }

protected:
  /// @brief See smoc_port_registry
  smoc_chan_out_base_if* createEntry()
    { return new entry_type(*this); }

  /// @brief See smoc_port_registry
  smoc_chan_in_base_if* createOutlet()
    { return new outlet_type(*this); }

private:
};

template <typename T>
class smoc_multiplex_fifo {
public:
  typedef smoc_multiplex_fifo<T>  this_type;
  typedef size_t FifoId;

  class VirtFifo: public smoc_multiplex_vfifo_chan<T>::chan_init {
  public:
    typedef T                             data_type;
    typedef smoc_multiplex_vfifo_chan<T>  chan_type;
    
    friend class smoc_multiplex_fifo<T>;
  
  private:
    VirtFifo(const p_smoc_multiplex_fifo_chan &pSharedFifoMem, size_t m)
      : smoc_multiplex_vfifo_chan<T>::chan_init("", pSharedFifoMem, m)
    {}
    VirtFifo(const std::string& name, const p_smoc_multiplex_fifo_chan &pSharedFifoMem, size_t m)
      : smoc_multiplex_vfifo_chan<T>::chan_init(name, pSharedFifoMem, m)
    {}
  public:
    this_type &operator<<(const T &x) {
      add(x);
      pSharedFifoMem->produce(this->fifoId, 1);
      return *this;
    }
  };

private:
  size_t  n;  // size of the shared fifo memory
  size_t  m;  // out of order access, zero is no out of order

  p_smoc_multiplex_fifo_chan pSharedFifoMem;

public:
  /// @param n size of the shared fifo memory
  /// @param m out of order access, zero is no out of order
  smoc_multiplex_fifo(size_t n = 1, size_t m = 0)
    : n(n), m(m), pSharedFifoMem(new smoc_multiplex_fifo_chan("", n, m)) {}
  smoc_multiplex_fifo(const std::string& name, size_t n = 1, size_t m = 0)
    : n(n), m(m), pSharedFifoMem(new smoc_multiplex_fifo_chan(name, n, m)) {}

  VirtFifo getVirtFifo()
    { return VirtFifo(pSharedFifoMem, m); }
  
  VirtFifo getVirtFifo(const std::string& name)
    { return VirtFifo(name, pSharedFifoMem, m); }
};

#endif // _INCLUDED_SMOC_MULTIPLEX_FIFO_HPP
