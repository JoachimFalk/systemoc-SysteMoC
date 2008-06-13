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

#include <utility>

#include <CoSupport/commondefs.h>

#include <systemoc/smoc_config.h>

#include "detail/smoc_root_chan.hpp"
#include "smoc_chan_if.hpp"
#include "smoc_storage.hpp"
#include "smoc_chan_adapter.hpp"
#include "smoc_fifo.hpp"
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

// FIX possibly broken offsetof from stddef.h
#undef offsetof

/* Offset of member MEMBER in a struct of type TYPE. */
#ifndef __cplusplus
# define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#else
/* The cast to "char &" below avoids problems with user-defined
 *    "operator &", which can appear in a POD type.  */
# define offsetof(TYPE, MEMBER)                                 \
  (&reinterpret_cast<const volatile char &>                     \
    (reinterpret_cast<TYPE *>(4711)->MEMBER) -                  \
   reinterpret_cast<const volatile char *>(4711))
#endif /* C++ */

class smoc_multiplex_vfifo_chan_base;

template <class T, class A> class smoc_multiplex_fifo_entry;
template <class T, class A> class smoc_multiplex_fifo_outlet;
template <class T, class A> class smoc_multiplex_vfifo_entry;
template <class T, class A> class smoc_multiplex_vfifo_outlet;

class smoc_multiplex_fifo_chan_base
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
  typedef smoc_multiplex_fifo_chan_base this_type;

  friend class smoc_multiplex_vfifo_chan_base;
  template <class T, class A> friend class smoc_multiplex_fifo_entry;
  template <class T, class A> friend class smoc_multiplex_fifo_outlet;
  template <class T, class A> friend class smoc_multiplex_vfifo_entry;
  template <class T, class A> friend class smoc_multiplex_vfifo_outlet;
public:
  /// @brief Channel initializer
  class chan_init {
    friend class smoc_multiplex_fifo_chan_base;
  protected:
    std::string name; // Channel name
    size_t      n;    // Size of the shared fifo memory
    size_t      m;    // Out of order access, zero is no out of order
  protected:
    chan_init(const std::string &name, size_t n, size_t m)
      : name(name), n(n), m(m) {}
  };

  typedef size_t FifoId;
  typedef std::list<FifoId> FifoSequence;
  typedef std::map<FifoId, smoc_multiplex_vfifo_chan_base *> FifoMap;
private:
  FifoId        fifoIdCount;  // For virtual fifo enumeration
  FifoMap       vFifos;
  FifoSequence  fifoSequence;
  FifoSequence  fifoSequenceOOO;
  const size_t  fifoOutOfOrder; // == 0 => no out of order access only one element visible

  // This are the EventMapManager for the plain fifo access operations
  Detail::EventMapManager emmFree;      // for smoc_multiplex_fifo_entry
  Detail::EventMapManager emmAvailable; // for smoc_multiplex_fifo_outlet
#ifdef SYSTEMOC_ENABLE_VPC
  Detail::LatencyQueue  latencyQueue;
  Detail::DIIQueue      diiQueue;
#endif

  /// @brief The tokenId of the next commit token
  size_t tokenId;
protected:
  smoc_multiplex_fifo_chan_base(const chan_init &i);

  void registerVFifo(smoc_multiplex_vfifo_chan_base *vfifo);
  void deregisterVFifo(smoc_multiplex_vfifo_chan_base *vfifo);

#ifdef SYSTEMOC_ENABLE_VPC
  void commitRead(size_t n, const smoc_ref_event_p &diiEvent)
#else
  void commitRead(size_t n)
#endif
  {
#ifdef SYSTEMOC_TRACE
    TraceLog.traceCommExecIn(this, n);
#endif
    rpp(n);
    emmAvailable.decreasedCount(visibleCount());
#ifdef SYSTEMOC_ENABLE_VPC
    // Delayed call of diiExpired(n);
    diiQueue.addEntry(n, diiEvent);
#else
    diiExpired(n);
#endif
  }

#ifdef SYSTEMOC_ENABLE_VPC
  void consume(FifoId from, size_t n, const smoc_ref_event_p &diiEvent);
#else
  void consume(FifoId from, size_t n);
#endif

  void diiExpired(size_t n);
 
  /// @brief See smoc_chan_in_base_if
  smoc_event &dataAvailableEvent(size_t n)
    { return emmAvailable.getEvent(visibleCount(), n); }

#ifdef SYSTEMOC_ENABLE_VPC
  void commitWrite(size_t n, const smoc_ref_event_p &latEvent)
#else
  void commitWrite(size_t n)
#endif
  {
#ifdef SYSTEMOC_TRACE
    TraceLog.traceCommExecOut(this, n);
#endif
    tokenId += n;
    wpp(n);
    emmFree.decreasedCount(freeCount());
#ifdef SYSTEMOC_ENABLE_VPC
    // Delayed call of latencyExpired(n);
    latencyQueue.addEntry(n, latEvent);
#else
    latencyExpired(n);
#endif
  }

#ifdef SYSTEMOC_ENABLE_VPC
  void produce(FifoId to, size_t n, const smoc_ref_event_p &latEvent);
#else
  void produce(FifoId to, size_t n);
#endif
  
  /// @brief Detail::LatencyQueue::ILatencyExpired
  void latencyExpired(size_t n);

  smoc_event &spaceAvailableEvent(size_t n)
    { return emmFree.getEvent(freeCount(), n); }

  /// @brief See smoc_chan_out_base_if
  size_t outTokenId() const
    { return tokenId; }

  /// @brief See smoc_chan_in_base_if
  size_t inTokenId() const
    { return tokenId - usedCount(); }

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
};

/**
 * This class provides interfaces and connect methods
 */
template<class T, class A>
class smoc_multiplex_fifo_chan
: public smoc_fifo_storage<T, smoc_multiplex_fifo_chan_base>
{
  typedef smoc_multiplex_fifo_chan<T,A> this_type;

  friend class smoc_multiplex_fifo_outlet<T,A>;
  friend class smoc_multiplex_fifo_entry<T,A>;
  friend class smoc_multiplex_vfifo_entry<T,A>;
  friend class smoc_multiplex_vfifo_outlet<T,A>;
  template <class TT> friend class smoc_multiplex_vfifo_entry<T,A>::AccessImpl;
  template <class TT> friend class smoc_multiplex_vfifo_outlet<T,A>::AccessImpl;
public:
  typedef T                               data_type;
  typedef smoc_multiplex_fifo_entry<T,A>  entry_type;
  typedef smoc_multiplex_fifo_outlet<T,A> outlet_type;
  
  typedef typename entry_type::iface_type   entry_iface_type;
  typedef typename outlet_type::iface_type  outlet_iface_type;

  /// @brief Channel initializer
  typedef typename smoc_fifo_storage<T, smoc_multiplex_fifo_chan_base>::chan_init chan_init;

  /// @brief Constructor
  smoc_multiplex_fifo_chan(const chan_init &i)
    : smoc_fifo_storage<T, smoc_multiplex_fifo_chan_base>(i)
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
  smoc_chan_out_base_if *createEntry()
    { return new entry_type(*this); }

  /// @brief See smoc_port_registry
  smoc_chan_in_base_if *createOutlet()
    { return new outlet_type(*this); }
};

/**
 * This class implements the channel out interface
 */
template<class T, class A>
class smoc_multiplex_fifo_entry
: public smoc_chan_out_if<T,smoc_channel_access_if> {
  typedef smoc_multiplex_fifo_entry<T,A> this_type;
private:
  /// @brief The channel implementation
  smoc_multiplex_fifo_chan<T,A> &chan;
public:
  /// @brief Constructor
  smoc_multiplex_fifo_entry(smoc_multiplex_fifo_chan<T,A> &chan)
    : chan(chan) {}
protected:
  /// @brief See smoc_chan_out_base_if
#ifdef SYSTEMOC_ENABLE_VPC
  void commitWrite(size_t n, const smoc_ref_event_p &latEvent)
    { return chan.commitWrite(n, latEvent); }
#else
  void commitWrite(size_t n)
    { return chan.commitWrite(n); }
#endif

  /// @brief See smoc_chan_out_base_if
  smoc_event &spaceAvailableEvent(size_t n)
    { return chan.spaceAvailableEvent(n); }
 
  /// @brief See smoc_chan_out_base_if
  size_t numFree() const
    { return chan.freeCount(); }
 
  /// @brief See smoc_chan_out_base_if
  size_t outTokenId() const
    { return chan.outTokenId(); }

  /// @brief See smoc_chan_out_if
  typename this_type::access_type *getWriteChannelAccess()
    { return chan.getWriteChannelAccess(); }
};

/**
 * This class implements the channel in interface
 */
template<class T, class A>
class smoc_multiplex_fifo_outlet
: public smoc_chan_in_if<T,smoc_channel_access_if> {
  typedef smoc_multiplex_fifo_outlet<T,A> this_type;
private:
  /// @brief The channel implementation
  smoc_multiplex_fifo_chan<T,A> &chan;
public:
  /// @brief Constructor
  smoc_multiplex_fifo_outlet(smoc_multiplex_fifo_chan<T,A> &chan)
    : chan(chan) {}
protected:
  /// @brief See smoc_chan_in_base_if
#ifdef SYSTEMOC_ENABLE_VPC
  void commitRead(size_t n, const smoc_ref_event_p &diiEvent)
    { return chan.commitRead(n, diiEvent); }
#else
  void commitRead(size_t n)
    { return chan.commitRead(n); }
#endif

  /// @brief See smoc_chan_in_base_if
  smoc_event &dataAvailableEvent(size_t n)
    { return chan.dataAvailableEvent(n); }

  /// @brief See smoc_chan_in_base_if
  size_t numAvailable() const
    { return chan.visibleCount(); }

  /// @brief See smoc_chan_in_base_if
  size_t inTokenId() const
    { return chan.inTokenId(); }

  /// @brief See smoc_chan_in_if
  typename this_type::access_type *getReadChannelAccess()
    { return chan.getReadChannelAccess(); }
};

typedef boost::shared_ptr<smoc_multiplex_fifo_chan_base>  p_smoc_multiplex_fifo_chan;

template <class T, class A> class smoc_multiplex_vfifo_entry;
template <class T, class A> class smoc_multiplex_vfifo_outlet;

class smoc_multiplex_vfifo_chan_base
: private boost::noncopyable,
  // due to out of order access we always need a visible area management
  public Detail::QueueRVWPtr 
{
  typedef smoc_multiplex_vfifo_chan_base  this_type;

  friend class smoc_multiplex_fifo_chan_base;
  template <class T, class A> friend class smoc_multiplex_vfifo_entry;
  template <class T, class A> friend class smoc_multiplex_vfifo_outlet;
public:
  typedef size_t FifoId;

  /// @brief Channel initializer
  class chan_init {
  public:
    friend class smoc_multiplex_vfifo_chan_base;
  protected:
    chan_init(const std::string& name, const p_smoc_multiplex_fifo_chan &pChanImpl, size_t m)
      : name(name), pChanImpl(pChanImpl),
        fifoId(pChanImpl->getNewFifoId()), m(m)
    {}
  private:
    std::string                 name;
    p_smoc_multiplex_fifo_chan  pChanImpl;
    FifoId                      fifoId;
    size_t                      m;
  };

private:
  Detail::EventMapManager emmAvailable;
  FifoId fifoId;
  p_smoc_multiplex_fifo_chan pChanImpl;
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
};

template<class T, class A>
class smoc_multiplex_vfifo_chan
: public smoc_multiplex_vfifo_chan_base,
  public smoc_port_registry
{
  typedef smoc_multiplex_vfifo_chan<T,A>  this_type;
  typedef smoc_multiplex_vfifo_chan_base  base_type;

  friend class smoc_multiplex_vfifo_outlet<T,A>;
  friend class smoc_multiplex_vfifo_entry<T,A>;
public:
  typedef T                                 data_type;
  typedef smoc_multiplex_vfifo_entry<T,A>   entry_type;
  typedef smoc_multiplex_vfifo_outlet<T,A>  outlet_type;
  
  typedef typename entry_type::iface_type   entry_iface_type;
  typedef typename outlet_type::iface_type  outlet_iface_type;

  typedef smoc_multiplex_fifo_chan<T,A>       MultiplexChannel;
  typedef boost::shared_ptr<MultiplexChannel> PMultiplexChannel;

  /// @brief Channel initializer
  class chan_init: public base_type::chan_init {
    friend class smoc_multiplex_vfifo_chan<T,A>;
  public:
    typedef T                               data_type;
    typedef smoc_multiplex_vfifo_chan<T,A>  chan_type;
  private:
    PMultiplexChannel pMultiplexChan;
  public:
    chan_init(const PMultiplexChannel &pMultiplexChan, size_t m)
      : base_type::chan_init("", pMultiplexChan, m),
        pMultiplexChan(pMultiplexChan)
      {}
    chan_init(const std::string& name, const PMultiplexChannel &pMultiplexChan, size_t m)
      : base_type::chan_init(name, pMultiplexChan, m),
        pMultiplexChan(pMultiplexChan)
      {}

    this_type &operator<<(const T &x) {
      pMultiplexChan->storage[pMultiplexChan->windex] = std::make_pair(this->fifoId, x);
      pMultiplexChan->produce(this->fifoId, 1);
      return *this;
    }
  };
private:
  PMultiplexChannel pMultiplexChan;
public:
  /// @brief Constructor
  smoc_multiplex_vfifo_chan(const chan_init &i)
    : base_type(i), pMultiplexChan(i.pMultiplexChan)
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

template<class T, class A>
class smoc_multiplex_vfifo_outlet
: public smoc_chan_in_if<T,smoc_channel_access_if> {
  typedef smoc_multiplex_vfifo_outlet<T,A> this_type;
  // Ugh need this friend decl for the AccessImpl friend decl in
  // smoc_multiplex_fifo_chan
  friend class smoc_multiplex_fifo_chan<T,A>;
public:
  typedef smoc_multiplex_fifo_chan<T,A> MultiplexChannel;

  template<class TT>
  class AccessImpl: public this_type::access_type {
    typedef AccessImpl<TT> this_type;
  public:
    typedef smoc_multiplex_vfifo_outlet<T,A>  ChanIfImpl;
    typedef typename this_type::return_type   return_type;
  private:
#ifndef NDEBUG
    size_t limit;
#endif
  private:
    ChanIfImpl &getChanIfImpl() {
      std::cerr << "offsetof(ChanIfImpl, accessImpl): " <<  offsetof(ChanIfImpl, accessImpl) << std::endl;

      ChanIfImpl *retval =
        reinterpret_cast<ChanIfImpl *>(
          reinterpret_cast<char *>(this) -
          offsetof(ChanIfImpl, accessImpl));
      std::cerr << "this: " << this << ", retval: " << retval << std::endl;
      return *retval;
    }
    MultiplexChannel &getChan()
      { return *getChanIfImpl().chan.pMultiplexChan.get(); }
  public:
    AccessImpl()
#ifndef NDEBUG
      : limit(0)
#endif
      {}

#ifndef NDEBUG
    void setLimit(size_t n)
      { limit = n; }
#endif

    bool tokenIsValid(size_t n) const {
      assert(n < limit);
      return true;
    }

    // Access methods
    return_type operator[](size_t n) {
      assert(n < limit);
      std::cerr << "smoc_multiplex_vfifo_outlet<T,A>::AccessImpl<TT>::operator[](size_t) BEGIN" << std::endl;
      assert(n < limit);
      MultiplexChannel &chan = getChan();
      
      size_t rindex;
      
      std::cerr << "XXX " << getChanIfImpl().chan.fifoId << std::endl;
      
      for (rindex = chan.rIndex();
           n >= 1 && A::get(chan.storage[rindex].get()) != getChanIfImpl().chan.fifoId;
           rindex = rindex < chan.fSize() - 1 ? rindex + 1 : 0)
        if (A::get(chan.storage[rindex].get()) == getChanIfImpl().chan.fifoId)
          --n;
      std::cerr << "smoc_multiplex_vfifo_outlet<T,A>::AccessImpl<TT>::operator[](size_t) END" << std::endl;
      return chan.storage[rindex];
    }
    const return_type operator[](size_t n) const
      { return const_cast<this_type *>(this)->operator[](n); }
  };
private:
  /// @brief The channel implementation
  smoc_multiplex_vfifo_chan<T,A> &chan;
  AccessImpl<T>                   accessImpl;
public:
  /// @brief Constructor
  smoc_multiplex_vfifo_outlet(smoc_multiplex_vfifo_chan<T,A> &chan)
    : chan(chan) {}
protected:
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
    chan.pChanImpl->consume(chan.fifoId, consume, diiEvent);
#else
    chan.pChanImpl->consume(chan.fifoId, consume);
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

  /// @brief See smoc_chan_in_if
  AccessImpl<T> *getReadChannelAccess()
    { return &accessImpl; }
};

template<class T, class A>
class smoc_multiplex_vfifo_entry
: public smoc_chan_out_if<T,smoc_channel_access_if> {
  typedef smoc_multiplex_vfifo_entry<T,A> this_type;
  // Ugh need this friend decl for the AccessImpl friend decl in
  // smoc_multiplex_fifo_chan
  friend class smoc_multiplex_fifo_chan<T,A>;
private:
  typedef smoc_multiplex_fifo_chan<T,A> MultiplexChannel;

  template<class TT>
  class AccessImpl: public this_type::access_type {
    typedef AccessImpl<TT>  this_type;
  public:
    typedef smoc_multiplex_vfifo_entry<T,A> ChanIfImpl;
    typedef typename this_type::return_type return_type;
  private:
#ifndef NDEBUG
    size_t limit;
#endif
  private:
    ChanIfImpl &getChanIfImpl() {
      std::cerr << "offsetof(ChanIfImpl, accessImpl): " <<  offsetof(ChanIfImpl, accessImpl) << std::endl;

      ChanIfImpl *retval =
        reinterpret_cast<ChanIfImpl *>(
          reinterpret_cast<char *>(this) -
          offsetof(ChanIfImpl, accessImpl));
      std::cerr << "this: " << this << ", retval: " << retval << std::endl;
      return *retval;
    }
    MultiplexChannel &getChan()
      { return *getChanIfImpl().chan.pMultiplexChan.get(); }
  public:
    AccessImpl()
#ifndef NDEBUG
      : limit(0)
#endif
      {}

#ifndef NDEBUG
    void setLimit(size_t n)
      { limit = n; }
#endif

    bool tokenIsValid(size_t n) const {
      assert(n < limit);
      return true;
    }

    // Access methods
    return_type operator[](size_t n) {
      std::cerr << "smoc_multiplex_vfifo_entry<T,A>::AccessImpl<TT>::operator[](size_t) BEGIN" << std::endl;
      assert(n < limit);
      MultiplexChannel &chan = getChan();
      size_t windex = chan.wIndex() + n;
      if (windex >= chan.fSize())
        windex -= chan.fSize();
      std::cerr << "smoc_multiplex_vfifo_entry<T,A>::AccessImpl<TT>::operator[](size_t) END" << std::endl;
      return chan.storage[chan.wIndex() + n];
    }

    const return_type operator[](size_t n) const
      { return const_cast<this_type *>(this)->operator[](n); }
  };
private:
  /// @brief The channel implementation
  smoc_multiplex_vfifo_chan<T,A> &chan;
  AccessImpl<T>                   accessImpl;
public:
  /// @brief Constructor
  smoc_multiplex_vfifo_entry(smoc_multiplex_vfifo_chan<T,A> &chan)
    : chan(chan) {}
protected:
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
    chan.pChanImpl->produce(chan.fifoId, produce, latEvent); 
#else
    chan.pChanImpl->produce(chan.fifoId, produce); 
#endif
  }

  /// @brief See smoc_chan_out_base_if
  smoc_event &spaceAvailableEvent(size_t n)
    { return chan.pChanImpl->spaceAvailableEvent(n); }

  /// @brief See smoc_chan_out_base_if
  size_t numFree() const
    { return chan.freeCount(); }

  /// @brief See smoc_chan_out_base_if
  size_t outTokenId() const
    { return chan.tokenId; }

  /// @brief See smoc_chan_out_if
  AccessImpl<T> *getWriteChannelAccess()
    { return &accessImpl; }
};

template <typename T, typename A>
class smoc_multiplex_fifo
: public smoc_multiplex_fifo_chan<T,A>::chan_init {
  typedef smoc_multiplex_fifo<T,A> this_type;
private:
  typedef typename smoc_multiplex_fifo_chan<T,A>::chan_init base_type;
public:
  typedef smoc_multiplex_fifo_chan<T,A> chan_type;

  typedef size_t FifoId;

  typedef boost::shared_ptr<chan_type> PChannel;

private:
  PChannel pChanImpl;
public:
  /// @param n size of the shared fifo memory
  /// @param m out of order access, zero is no out of order
  smoc_multiplex_fifo(size_t n = 1, size_t m = 0)
    : base_type("", n, m),
      pChanImpl(new chan_type(*this))
    {}
  smoc_multiplex_fifo(const std::string &name, size_t n = 1, size_t m = 0)
    : base_type(name, n, m),
      pChanImpl(new chan_type(*this))
    {}

  typename smoc_multiplex_vfifo_chan<T,A>::chan_init getVirtFifo()
    { return typename smoc_multiplex_vfifo_chan<T,A>::chan_init(pChanImpl, this->m); }
  
  typename smoc_multiplex_vfifo_chan<T,A>::chan_init getVirtFifo(const std::string& name)
    { return typename smoc_multiplex_vfifo_chan<T,A>::chan_init(this->name, pChanImpl, this->m); }
};

#endif // _INCLUDED_SMOC_MULTIPLEX_FIFO_HPP
