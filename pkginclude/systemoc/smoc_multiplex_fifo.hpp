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
#include <boost/function.hpp>

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
  typedef std::map<FifoId, const boost::function<void (size_t)> > FifoMap;
protected:
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
protected:
  smoc_multiplex_fifo_chan_base(const chan_init &i);

  void registerVFifo(const FifoMap::value_type &entry);
  void deregisterVFifo(FifoId fifoId);

#ifdef SYSTEMOC_ENABLE_VPC
  void commitRead(size_t n, const smoc_ref_event_p &diiEvent);
#else
  void commitRead(size_t n);
#endif

  void diiExpired(size_t n);
 
  /// @brief See smoc_chan_in_base_if
  smoc_event &dataAvailableEvent(size_t n)
    { return emmAvailable.getEvent(visibleCount(), n); }

#ifdef SYSTEMOC_ENABLE_VPC
  void commitWrite(size_t n, const smoc_ref_event_p &latEvent);
#else
  void commitWrite(size_t n);
#endif

  /// @brief Detail::LatencyQueue::ILatencyExpired
  void latencyExpired(size_t n);

  smoc_event &spaceAvailableEvent(size_t n)
    { return emmFree.getEvent(freeCount(), n); }

  /// @brief See smoc_chan_in_base_if
  size_t inTokenId() const
    { return -1; }

  /// @brief See smoc_root_chan
  void assemble(smoc_modes::PGWriter &pgw) const
    {}
  
  /// @brief See smoc_root_chan
  void channelContents(smoc_modes::PGWriter &pgw) const
    { assert(0); }
  
  /// @brief See smoc_root_chan
  virtual void channelAttributes(smoc_modes::PGWriter &pgw) const
    { assert(0); }
};

/**
 * This class provides interfaces and connect methods
 */
template<class T, class A>
class smoc_multiplex_fifo_chan
: public smoc_fifo_storage<T, smoc_multiplex_fifo_chan_base>
{
  typedef smoc_multiplex_fifo_chan<T,A>                       this_type;
  typedef smoc_fifo_storage<T, smoc_multiplex_fifo_chan_base> base_type;

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

  typedef typename this_type::FifoId        FifoId;
  typedef typename this_type::FifoSequence  FifoSequence;
  typedef typename this_type::FifoMap       FifoMap;

  /// @brief Channel initializer
  typedef typename smoc_fifo_storage<T, smoc_multiplex_fifo_chan_base>::chan_init chan_init;

  /// @brief Constructor
  smoc_multiplex_fifo_chan(const chan_init &i)
    : smoc_fifo_storage<T, smoc_multiplex_fifo_chan_base>(i)
  {}

#ifdef SYSTEMOC_ENABLE_VPC
  void commitWrite(size_t n, const smoc_ref_event_p &latEvent)
#else
  void commitWrite(size_t n)
#endif
  {
    size_t windex = this->wIndex();
    size_t fsize  = this->fSize();
    
    for (size_t i = 0; i < n; ++i) {
      FifoId to = A::get(this->storage[windex].get());
      this->fifoSequence.push_back(to);
      if (++windex >= fsize)
        windex -= fsize;
    }
    
#ifdef SYSTEMOC_ENABLE_VPC
    base_type::commitWrite(n, latEvent);
#else
    base_type::commitWrite(n);
#endif
  }

#ifdef SYSTEMOC_ENABLE_VPC
  void produce(FifoId to, size_t n, const smoc_ref_event_p &latEvent)
#else
  void produce(FifoId to, size_t n)
#endif
  {
    size_t windex = this->wIndex();
    size_t fsize  = this->fSize();
    
    for (size_t i = 0; i < n; ++i) {
      A::put(this->storage[windex].get(), to);
      this->fifoSequence.push_back(to);
      if (++windex >= fsize)
        windex -= fsize;
    }
    
#ifdef SYSTEMOC_ENABLE_VPC
    base_type::commitWrite(n, latEvent);
#else
    base_type::commitWrite(n);
#endif
  }

#ifdef SYSTEMOC_ENABLE_VPC
  void consume(FifoId from, size_t n, const smoc_ref_event_p &diiEvent)
#else
  void consume(FifoId from, size_t n)
#endif
  {
  //std::cerr << "smoc_multiplex_fifo_chan_base::consume(" << from << ", " << n << ") [BEGIN]" << std::endl;
  //std::cerr << "fifoOutOfOrder == " << fifoOutOfOrder << std::endl;
  //std::cerr << "freeCount():    " << freeCount() << std::endl;
  //std::cerr << "usedCount():    " << usedCount() << std::endl;
  //std::cerr << "visibleCount(): " << visibleCount() << std::endl;
  
    size_t dindex;
    
    // Find n'th fifoId == from element in storage
    for (dindex = this->rIndex();
         n > 1 || A::get(this->storage[dindex].get()) != from;
         dindex = dindex < this->fSize() - 1 ? dindex + 1 : 0)
      if (A::get(this->storage[dindex].get()) == from)
        --n;
    // The found fifoId == from element and all previous elements must be
    // consumed
    
    for (size_t sindex = dindex; sindex != this->rIndex();) {
      sindex = (sindex == 0 ? this->fSize() : sindex) - 1;
      if (A::get(this->storage[sindex].get()) != from) {
        this->storage[dindex].put(this->storage[sindex].get());
        do {
          dindex = (dindex == 0 ? this->fSize() : dindex) - 1;
        } while (A::get(this->storage[dindex].get()) != from);
      }
    }
    
#ifdef SYSTEMOC_ENABLE_VPC
    base_type::commitRead(n, diiEvent);
#else
    base_type::commitRead(n);
#endif
    
    for (typename FifoSequence::iterator iter = this->fifoSequenceOOO.begin();
         n > 0;
         ) {
      assert(iter !=  this->fifoSequenceOOO.end());
      if (*iter == from) {
        if (this->visibleCount() >= this->fifoOutOfOrder + n) {
          FifoId fId = this->fifoSequence.front();
          typename FifoMap::iterator fIter = this->vFifos.find(fId);
          this->fifoSequence.pop_front();
          this->fifoSequenceOOO.push_back(fId);
          fIter->second(1);
        }
        iter = this->fifoSequenceOOO.erase(iter); --n;
      } else {
        ++iter;
      }
    }
    
  //std::cerr << "smoc_multiplex_fifo_chan_base::consume(" << from << ", " << n << ") [END]" << std::endl;
  //std::cerr << "freeCount():    " << freeCount() << std::endl;
  //std::cerr << "usedCount():    " << usedCount() << std::endl;
  //std::cerr << "visibleCount(): " << visibleCount() << std::endl;
  }

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
    { return -1; }

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
    { return -1; }

  /// @brief See smoc_chan_in_if
  typename this_type::access_type *getReadChannelAccess()
    { return chan.getReadChannelAccess(); }
};

template<class T, class A>
class smoc_multiplex_vfifo_outlet
: public smoc_chan_in_if<T,smoc_channel_access_if> {
  typedef smoc_multiplex_vfifo_outlet<T,A> this_type;
  // Ugh need this friend decl for the AccessImpl friend decl in
  // smoc_multiplex_fifo_chan
  friend class smoc_multiplex_fifo_chan<T,A>;
public:
  typedef smoc_multiplex_fifo_chan<T,A>       MultiplexChannel;
  typedef boost::shared_ptr<MultiplexChannel> PMultiplexChannel;
  typedef typename MultiplexChannel::FifoId   FifoId;

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
      { return *getChanIfImpl().chan.get(); }
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
      
      std::cerr << "XXX " << getChanIfImpl().fifoId << std::endl;
      
      for (rindex = chan.rIndex();
           n >= 1 && A::get(chan.storage[rindex].get()) != getChanIfImpl().fifoId;
           rindex = rindex < chan.fSize() - 1 ? rindex + 1 : 0)
        if (A::get(chan.storage[rindex].get()) == getChanIfImpl().fifoId)
          --n;
      std::cerr << "smoc_multiplex_vfifo_outlet<T,A>::AccessImpl<TT>::operator[](size_t) END" << std::endl;
      return chan.storage[rindex];
    }
    const return_type operator[](size_t n) const
      { return const_cast<this_type *>(this)->operator[](n); }
  };
private:
  /// @brief The channel implementation
  PMultiplexChannel       chan;
  FifoId                  fifoId;
  size_t                  countAvailable;
  Detail::EventMapManager emmAvailable;
  AccessImpl<T>           accessImpl;
public:
  /// @brief Constructor
  smoc_multiplex_vfifo_outlet(const PMultiplexChannel &chan, FifoId fifoId)
    : chan(chan), fifoId(fifoId), countAvailable(0) {
    chan->registerVFifo(std::make_pair(
      fifoId,
      std::bind1st(std::mem_fun(&this_type::latencyExpired), this)
    ));
  }

  ~smoc_multiplex_vfifo_outlet() {
    chan->deregisterVFifo(fifoId);
  }
protected:
  /// @brief See smoc_chan_in_base_if
#ifdef SYSTEMOC_ENABLE_VPC
  void commitRead(size_t n, const smoc_ref_event_p &diiEvent)
#else
  void commitRead(size_t n)
#endif
  {
    assert(countAvailable >= n);
    countAvailable -= n;
    emmAvailable.decreasedCount(countAvailable);
#ifdef SYSTEMOC_ENABLE_VPC
    chan->consume(fifoId, n, diiEvent);
#else
    chan->consume(fifoId, n);
#endif
  }

  /// @brief See smoc_chan_in_base_if
  smoc_event &dataAvailableEvent(size_t n)
    { return emmAvailable.getEvent(countAvailable, n); }

  /// @brief See smoc_chan_in_base_if
  size_t numAvailable() const
    { return countAvailable; }

  /// @brief See smoc_chan_in_base_if
  size_t inTokenId() const
    { return -1; }

  /// @brief See smoc_chan_in_if
  AccessImpl<T> *getReadChannelAccess()
    { return &accessImpl; }

  void latencyExpired(size_t n) {
    countAvailable += n;
    emmAvailable.increasedCount(countAvailable);
  }
};

template<class T, class A>
class smoc_multiplex_vfifo_entry
: public smoc_chan_out_if<T,smoc_channel_access_if> {
  typedef smoc_multiplex_vfifo_entry<T,A> this_type;
  // Ugh need this friend decl for the AccessImpl friend decl in
  // smoc_multiplex_fifo_chan
  friend class smoc_multiplex_fifo_chan<T,A>;
private:
  typedef smoc_multiplex_fifo_chan<T,A>       MultiplexChannel;
  typedef boost::shared_ptr<MultiplexChannel> PMultiplexChannel;
  typedef typename MultiplexChannel::FifoId   FifoId;

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
      { return *getChanIfImpl().chan.get(); }
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
  PMultiplexChannel chan;
  FifoId            fifoId;
  AccessImpl<T>     accessImpl;
public:
  /// @brief Constructor
  smoc_multiplex_vfifo_entry(const PMultiplexChannel &chan, FifoId fifoId)
    : chan(chan), fifoId(fifoId) {}
protected:
  /// @brief See smoc_chan_out_base_if
#ifdef SYSTEMOC_ENABLE_VPC
  void commitWrite(size_t n, const smoc_ref_event_p &latEvent)
#else
  void commitWrite(size_t n)
#endif
  {
    // This will do a callback to latencyExpired(produce) at the appropriate time
#ifdef SYSTEMOC_ENABLE_VPC
    chan->produce(fifoId, n, latEvent); 
#else
    chan->produce(fifoId, n); 
#endif
  }

  /// @brief See smoc_chan_out_base_if
  smoc_event &spaceAvailableEvent(size_t n)
    { return chan->spaceAvailableEvent(n); }

  /// @brief See smoc_chan_out_base_if
  size_t numFree() const
    { return chan->freeCount(); }

  /// @brief See smoc_chan_out_base_if
  size_t outTokenId() const
    { return -1; }

  /// @brief See smoc_chan_out_if
  AccessImpl<T> *getWriteChannelAccess()
    { return &accessImpl; }
};

template<class T, class A>
class smoc_multiplex_vfifo_chan
: private boost::noncopyable,
  public smoc_port_registry
{
  typedef smoc_multiplex_vfifo_chan<T,A>  this_type;

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
  typedef typename MultiplexChannel::FifoId   FifoId;

  /// @brief Channel initializer
  class chan_init {
    friend class smoc_multiplex_vfifo_chan<T,A>;
  public:
    typedef T                               data_type;
    typedef smoc_multiplex_vfifo_chan<T,A>  chan_type;
  private:
    FifoId            fifoId;
    PMultiplexChannel pMultiplexChan;
  public:
    chan_init(FifoId fifoId, const PMultiplexChannel &pMultiplexChan)
      : fifoId(fifoId), pMultiplexChan(pMultiplexChan)
      {}

    this_type &operator<<(const T &x) {
      pMultiplexChan->storage[pMultiplexChan->wIndex()] = x;
      pMultiplexChan->produce(this->fifoId, 1);
      return *this;
    }
  };
private:
  FifoId            fifoId;
  PMultiplexChannel pMultiplexChan;
public:
  /// @brief Constructor
  smoc_multiplex_vfifo_chan(const chan_init &i)
    : fifoId(i.fifoId), pMultiplexChan(i.pMultiplexChan)
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
    { return new entry_type(pMultiplexChan, fifoId); }

  /// @brief See smoc_port_registry
  smoc_chan_in_base_if* createOutlet()
    { return new outlet_type(pMultiplexChan, fifoId); }

private:
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
  FifoId   fifoIdCount;  // For virtual fifo enumeration
  PChannel pMultiplexChan;
public:
  /// @param n size of the shared fifo memory
  /// @param m out of order access, zero is no out of order
  smoc_multiplex_fifo(size_t n = 1, size_t m = 0)
    : base_type("", n, m), fifoIdCount(0),
      pMultiplexChan(new chan_type(*this))
    {}
  smoc_multiplex_fifo(const std::string &name, size_t n = 1, size_t m = 0)
    : base_type(name, n, m), fifoIdCount(0),
      pMultiplexChan(new chan_type(*this))
    {}

  typename smoc_multiplex_vfifo_chan<T,A>::chan_init getVirtFifo()
    { return typename smoc_multiplex_vfifo_chan<T,A>::chan_init(fifoIdCount++, pMultiplexChan); }
};

#endif // _INCLUDED_SMOC_MULTIPLEX_FIFO_HPP
