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

#ifndef _INCLUDED_SMOC_MULTIPLEX_FIFO_HPP
#define _INCLUDED_SMOC_MULTIPLEX_FIFO_HPP

#include <utility>
#include <vector>
#include <queue>
#include <map>

#include <CoSupport/commondefs.h>

#include <systemc.h>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

#include <systemoc/smoc_config.h>

#ifdef SYSTEMOC_ENABLE_VPC
# include <systemcvpc/hscd_vpc_Director.h>
#endif //SYSTEMOC_ENABLE_VPC

#include "detail/smoc_root_chan.hpp"
#include "detail/smoc_chan_if.hpp"
#include "detail/smoc_storage.hpp"
#include "smoc_chan_adapter.hpp"
#include "smoc_fifo.hpp"
#include "detail/smoc_latency_queues.hpp"
#include "detail/smoc_ring_access.hpp"
#include "detail/ConnectProvider.hpp"
#include "detail/EventMapManager.hpp"
#include "detail/QueueRVWPtr.hpp"
#include "detail/QueueFRVWPtr.hpp"
#include "detail/hscd_tdsim_TraceLog.hpp"

#include <smoc/detail/DumpingInterfaces.hpp>

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
  typedef std::map<FifoId, const boost::function<void (size_t)> > VOutletMap;
protected:
  VOutletMap    vOutlets;
  const size_t  fifoOutOfOrder; // == 0 => no out of order access only one element visible

  // This are the EventMapManager for the plain fifo access operations
  Detail::EventMapManager emmFree;      // for smoc_multiplex_fifo_entry
  Detail::EventMapManager emmAvailable; // for smoc_multiplex_fifo_outlet
protected:
  smoc_multiplex_fifo_chan_base(const chan_init &i);

  void registerVOutlet(const VOutletMap::value_type &entry);
  void deregisterVOutlet(FifoId fifoId);

  /// @brief See smoc_port_in_base_if
  smoc_event &dataAvailableEvent(size_t n)
    { return emmAvailable.getEvent(visibleCount(), n); }

  smoc_event &spaceAvailableEvent(size_t n)
    { return emmFree.getEvent(freeCount(), n); }

  /// @brief See smoc_port_in_base_if
  size_t inTokenId() const
    { return static_cast<size_t>(-1); }
public:
#ifdef SYSTEMOC_ENABLE_SGX
  // FIXME: This should be protected for the SysteMoC user but accessible
  // for SysteMoC visitors
  virtual void dumpInitalTokens(SysteMoC::Detail::IfDumpingInitialTokens *it) = 0;
#endif // SYSTEMOC_ENABLE_SGX
};

template<class T, class A>
class smoc_multiplex_fifo;

/**
 * This class provides interfaces and connect methods
 */
template<class T, class A>
class smoc_multiplex_fifo_chan
: public smoc_fifo_storage<T, smoc_multiplex_fifo_chan_base>
{
  typedef smoc_multiplex_fifo_chan<T,A>                       this_type;
  typedef smoc_fifo_storage<T, smoc_multiplex_fifo_chan_base> base_type;

  friend class smoc_multiplex_fifo<T,A>;
  friend class smoc_multiplex_fifo_outlet<T,A>;
  friend class smoc_multiplex_fifo_entry<T,A>;
  friend class smoc_multiplex_vfifo_entry<T,A>;
  friend class smoc_multiplex_vfifo_outlet<T,A>;
  friend class smoc_multiplex_vfifo_entry<T,A>::AccessImpl;
  friend class smoc_multiplex_vfifo_outlet<T,A>::AccessImpl;
public:
  typedef T                               data_type;
  typedef smoc_multiplex_fifo_entry<T,A>  entry_type;
  typedef smoc_multiplex_fifo_outlet<T,A> outlet_type;
  
  typedef typename this_type::FifoId      FifoId;
  typedef typename this_type::VOutletMap  VOutletMap;

  /// @brief Channel initializer
  typedef typename smoc_fifo_storage<T, smoc_multiplex_fifo_chan_base>::chan_init chan_init;
private:
#ifdef SYSTEMOC_ENABLE_VPC
  Detail::LatencyQueue  latencyQueue;
  Detail::DIIQueue      diiQueue;
#endif
protected:
  /// @brief Constructor
  smoc_multiplex_fifo_chan(const chan_init &i)
    : smoc_fifo_storage<T, smoc_multiplex_fifo_chan_base>(i)
#ifdef SYSTEMOC_ENABLE_VPC
      ,latencyQueue(std::bind1st(std::mem_fun(&this_type::latencyExpired), this), this)
      ,diiQueue(std::bind1st(std::mem_fun(&this_type::diiExpired), this))
#endif
  {}

#ifdef SYSTEMOC_ENABLE_VPC
  void commitWrite(size_t n, const smoc_ref_event_p &latEvent)
#else
  void commitWrite(size_t n)
#endif
  {
    this->wpp(n);
    this->emmFree.decreasedCount(this->freeCount());
#ifdef SYSTEMOC_ENABLE_VPC
    // Delayed call of latencyExpired(n);
    latencyQueue.addEntry(n, latEvent);
#else
    latencyExpired(n);
#endif
  }

  void latencyExpired(size_t n) {
  //std::cerr << "smoc_multiplex_fifo_chan_base::latencyExpired(" << n << ") [BEGIN]" << std::endl;
  //std::cerr << "fifoOutOfOrder == " << fifoOutOfOrder << std::endl;
  //std::cerr << "freeCount():    " << freeCount() << std::endl;
  //std::cerr << "usedCount():    " << usedCount() << std::endl;
  //std::cerr << "visibleCount(): " << visibleCount() << std::endl;
    // This may be a dummy function then visibleCount has been increased by
    // wpp(n) in commitWrite
    this->vpp(n); 
    
    size_t vcount = this->visibleCount();
    
    assert(vcount >= n);
    
    if (!this->vOutlets.empty()) {
      /*
       * Example: Nothing todo
       *  
       *             fsize
       *   ____________^___________
       *  /     OOOO               \   
       * |FFFFFFVVVVVVXXXPPPPPPPFFFF|
       *        ^   ^    ^      ^
       *      rindex|  vindex windex
       *          xindex
       *          oindex
       *
       * Example: Have to call latencyExpired on vfifo outlet
       *          for the new tokens inside the OOO area
       *
       *             fsize
       *   ____________^___________
       *  /     OOOOOO             \   
       * |FFFFFFVVVVXXXXXPPPPPPPFFFF|
       *        ^   ^ ^  ^      ^
       *      rindex| |vindex windex
       *        xindex|
       *            oindex
       *
       *             fsize
       *   ____________^___________
       *  /     OOOOOOOOOOOOOO     \   
       * |FFFFFFVVVVXXXXXPPPPPPPFFFF|
       *        ^   ^    ^      ^
       *      rindex|  vindex windex
       *        xindex oindex
       *
       *  F:  The free space area of size (findex - windex - 1) % fsize
       *  VX: The visible token area of size (vindex - rindex) % fsize
       *  P:  The token which are still in the pipeline (latency not expired)
       *
       *  O:  The OOO area of size fifoOutOfOrder + 1
       *  X:  The tokens for which the latency has expired
       *
       */
      
      size_t fsize  = this->fSize();
      size_t rindex = this->rIndex();
      // Calculate xindex to point to the first token inside or one outside the
      // OOO area for which the latency has expired.
      size_t xindex = (rindex + std::min(this->fifoOutOfOrder + 1, vcount - n)) % fsize;
      // Calculate oindex to point to the first token either outside
      // the OOO area or outside the visible area.
      size_t oindex = (rindex + std::min(this->fifoOutOfOrder + 1, vcount)) % fsize;
      
      for (; xindex != oindex; xindex = (xindex + 1) % fsize) {
        FifoId fId = A::get(this->storage[xindex].get());
        typename this_type::VOutletMap::iterator fIter = this->vOutlets.find(fId);
        assert(fIter != this->vOutlets.end());
        fIter->second(1);
      }
    } else {
      this->emmAvailable.increasedCount(vcount);
    }
  //std::cerr << "smoc_multiplex_fifo_chan_base::latencyExpired(" << n << ") [END]" << std::endl;
  //std::cerr << "freeCount():    " << freeCount() << std::endl;
  //std::cerr << "usedCount():    " << usedCount() << std::endl;
  //std::cerr << "visibleCount(): " << visibleCount() << std::endl;
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
    
    typedef typename this_type::MG MG;
    
    /*
     *  
     *             fsize
     *   ____________^___________
     *  /     OOOOO              \   
     * |FFFFFFVCVCIVIIVPIPPIIPFFFF|
     *        ^  ^     ^      ^
     *     rindex|   vindex windex
     *        dindex
     *
     *  F: The free space area of size (findex - windex - 1) % fsize
     *  V: The visible token area of size (vindex - rindex) % fsize
     *  P: The token which are still in the pipeline (latency not expired)
     *
     *  O: The OOO area of size fifoOutOfOrder + 1
     *  C: The tokens which have to be consumed.
     *     These tokens have color <from>.
     *  I: The tokens in the fifo which have
     *     color <from> but are not C.
     */
    
    if (n > 0) {
      MG dindex(this->rIndex(), typename MG::M(this->fSize()));
      
      // Find n'th fifoId == from element in storage
      for (size_t mc = n; mc > 1; ++dindex) {
        // rindex <= dindex < vindex in modulo fsize arith
        assert(dindex.between(this->rIndex(), MG(this->vIndex(), typename MG::M(this->fSize())) - 1));
        if (A::get(this->storage[dindex.getValue()].get()) == from)
          --mc;
      }
      for (; A::get(this->storage[dindex.getValue()].get()) != from; ++dindex) {
        // rindex <= dindex < vindex in modulo fsize arith
        assert(dindex.between(this->rIndex(), MG(this->vIndex(), typename MG::M(this->fSize())) - 1));
      }
      // The found fifoId == from element and all previous elements must be
      // consumed
      for (MG sindex(dindex); sindex != this->rIndex(); ) {
        --sindex;
        // rindex <= sindex < dindex in modulo fsize arith
        assert(sindex.between(this->rIndex(), dindex - 1));
        if (A::get(this->storage[sindex.getValue()].get()) != from) {
          this->storage[dindex.getValue()].put(this->storage[sindex.getValue()].get());
          --dindex;
        }
      }
    }
    
    //
#ifdef SYSTEMOC_ENABLE_VPC
    this->commitRead(n, diiEvent);
#else
    this->commitRead(n);
#endif
    
    /*
     * Example: Nothing todo
     *  
     *             fsize
     *   ____________^___________
     *  /     OOOOOOOOO          \   
     * |FFFFCCVVVVPPPPPPPPPPPPFFFF|
     *        ^   ^           ^
     *     rindex vindex    windex
     *            oindex
     *            xindex
     *
     * Example: Have to call latencyExpired on vfifo outlet
     *          for the new tokens inside the OOO area
     *
     *             fsize
     *   ____________^___________
     *  /     OOOOOO             \   
     * |FFFFCCVVVVXXVVVPPPPPPPFFFF|
     *        ^   ^ ^  ^      ^
     *     rindex | |vindex windex
     *        xindex|
     *            oindex
     *
     *             fsize
     *   ____________^___________
     *  /     OOOOOO             \   
     * |FFFFCCVVVVXPPPPPPPPPPPFFFF|
     *        ^   ^^          ^
     *     rindex |vindex   windex
     *            |oindex
     *          xindex
     *
     *  FY: The free space area of size (findex - windex - 1) % fsize
     *  V:  The visible token area of size (vindex - rindex) % fsize
     *  P:  The token which are still in the pipeline (latency not expired)
     *
     *  O:  The OOO area of size fifoOutOfOrder + 1
     *  C:  The tokens which have just been consumed
     *  X:  The tokens which have just entered the OOO area
     *
     */
    
    size_t vcount = this->visibleCount();
    size_t fsize  = this->fSize();
    size_t rindex = this->rIndex();
    // Calculate xindex to point to the first token for entering the OOO area.
    size_t xindex = (rindex + std::min(this->fifoOutOfOrder + 1 - n, vcount)) % fsize;
    // Calculate oindex to point to the first token either outside
    // the OOO area or outside the visible area.
    size_t oindex = (rindex + std::min(this->fifoOutOfOrder + 1, vcount)) % fsize;
    
    for (; xindex != oindex; xindex = (xindex + 1) % fsize) {
      FifoId fId = A::get(this->storage[xindex].get());
      typename this_type::VOutletMap::iterator fIter = this->vOutlets.find(fId);
      assert(fIter != this->vOutlets.end());
      fIter->second(1);
    }
    
  //std::cerr << "smoc_multiplex_fifo_chan_base::consume(" << from << ", " << n << ") [END]" << std::endl;
  //std::cerr << "freeCount():    " << freeCount() << std::endl;
  //std::cerr << "usedCount():    " << usedCount() << std::endl;
  //std::cerr << "visibleCount(): " << visibleCount() << std::endl;
  }

#ifdef SYSTEMOC_ENABLE_VPC
  void commitRead(size_t n, const smoc_ref_event_p &diiEvent)
#else
  void commitRead(size_t n)
#endif
  {
    this->rpp(n);
    this->emmAvailable.decreasedCount(this->visibleCount());
#ifdef SYSTEMOC_ENABLE_VPC
    // Delayed call of diiExpired(n);
    diiQueue.addEntry(n, diiEvent);
#else
    diiExpired(n);
#endif
  }

  void diiExpired(size_t n) {
  //std::cerr << "smoc_multiplex_fifo_chan_base::diiExpired(" << n << ") [BEGIN]" << std::endl;
  //std::cerr << "fifoOutOfOrder == " << fifoOutOfOrder << std::endl;
  //std::cerr << "freeCount():    " << freeCount() << std::endl;
  //std::cerr << "usedCount():    " << usedCount() << std::endl;
  //std::cerr << "visibleCount(): " << visibleCount() << std::endl;

    this->fpp(n);
    this->emmFree.increasedCount(this->freeCount());

  //std::cerr << "smoc_multiplex_fifo_chan_base::diiExpired(" << n << ") [END]" << std::endl;
  //std::cerr << "freeCount():    " << freeCount() << std::endl;
  //std::cerr << "usedCount():    " << usedCount() << std::endl;
  //std::cerr << "visibleCount(): " << visibleCount() << std::endl;
  }
protected:
  /// @brief See smoc_port_registry
  smoc_port_out_base_if *createEntry()
    { return new entry_type(*this); }

  /// @brief See smoc_port_registry
  smoc_port_in_base_if *createOutlet()
    { return new outlet_type(*this); }
};

/**
 * This class implements the channel out interface
 */
template<class T, class A>
class smoc_multiplex_fifo_entry
: public smoc_port_out_if<T,smoc_1d_port_access_if> {
  typedef smoc_multiplex_fifo_entry<T,A> this_type;
private:
  /// @brief The channel implementation
  smoc_multiplex_fifo_chan<T,A> &chan;
public:
  /// @brief Constructor
  smoc_multiplex_fifo_entry(smoc_multiplex_fifo_chan<T,A> &chan)
    : chan(chan) {}
protected:
  /// @brief See smoc_port_out_base_if
#ifdef SYSTEMOC_ENABLE_VPC
  void commitWrite(size_t n, const smoc_ref_event_p &latEvent)
    { return chan.commitWrite(n, latEvent); }
#else
  void commitWrite(size_t n)
    { return chan.commitWrite(n); }
#endif

  /// @brief See smoc_port_out_base_if
  smoc_event &spaceAvailableEvent(size_t n)
    { return chan.spaceAvailableEvent(n); }
 
  /// @brief See smoc_port_out_base_if
  size_t numFree() const
    { return chan.freeCount(); }
 
  /// @brief See smoc_port_out_base_if
  size_t outTokenId() const
    { return static_cast<size_t>(-1); }

  /// @brief See smoc_port_out_if
  typename this_type::access_type *getWritePortAccess()
    { return chan.getWritePortAccess(); }
};

/**
 * This class implements the channel in interface
 */
template<class T, class A>
class smoc_multiplex_fifo_outlet
: public smoc_port_in_if<T,smoc_1d_port_access_if> {
  typedef smoc_multiplex_fifo_outlet<T,A> this_type;
private:
  /// @brief The channel implementation
  smoc_multiplex_fifo_chan<T,A> &chan;
public:
  /// @brief Constructor
  smoc_multiplex_fifo_outlet(smoc_multiplex_fifo_chan<T,A> &chan)
    : chan(chan) {}
protected:
  /// @brief See smoc_port_in_base_if
#ifdef SYSTEMOC_ENABLE_VPC
  void commitRead(size_t n, const smoc_ref_event_p &diiEvent)
    { return chan.commitRead(n, diiEvent); }
#else
  void commitRead(size_t n)
    { return chan.commitRead(n); }
#endif

  /// @brief See smoc_port_in_base_if
  smoc_event &dataAvailableEvent(size_t n)
    { return chan.dataAvailableEvent(n); }

  /// @brief See smoc_port_in_base_if
  size_t numAvailable() const
    { return chan.visibleCount(); }

  /// @brief See smoc_port_in_base_if
  size_t inTokenId() const
    { return -1; }

  /// @brief See smoc_port_in_if
  typename this_type::access_type *getReadPortAccess()
    { return chan.getReadPortAccess(); }
};

template<class T, class A>
class smoc_multiplex_vfifo_outlet
: public smoc_port_in_if<T,smoc_1d_port_access_if> {
  typedef smoc_multiplex_vfifo_outlet<T,A> this_type;
  // Ugh need this friend decl for the AccessImpl friend decl in
  // smoc_multiplex_fifo_chan
  friend class smoc_multiplex_fifo_chan<T,A>;
public:
  typedef smoc_multiplex_fifo_chan<T,A>       MultiplexChannel;
  typedef boost::shared_ptr<MultiplexChannel> PMultiplexChannel;
  typedef typename MultiplexChannel::FifoId   FifoId;

  class AccessImpl: public this_type::access_type {
    typedef AccessImpl this_type;
  public:
    typedef smoc_multiplex_vfifo_outlet<T,A>  ChanIfImpl;
    typedef typename this_type::return_type   return_type;
  private:
#if defined(SYSTEMOC_ENABLE_DEBUG)
    size_t limit;
#endif
  private:
    ChanIfImpl &getChanIfImpl() {
      //std::cerr << "offsetof(ChanIfImpl, accessImpl): " <<  offsetof(ChanIfImpl, accessImpl) << std::endl;

      ChanIfImpl *retval =
        reinterpret_cast<ChanIfImpl *>(
          reinterpret_cast<char *>(this) -
          offsetof(ChanIfImpl, accessImpl));
      //std::cerr << "this: " << this << ", retval: " << retval << std::endl;
      return *retval;
    }
    MultiplexChannel &getChan()
      { return *getChanIfImpl().chan.get(); }
  public:
    AccessImpl()
#if defined(SYSTEMOC_ENABLE_DEBUG)
      : limit(0)
#endif
      {}

#if defined(SYSTEMOC_ENABLE_DEBUG)
    void setLimit(size_t n)
      { limit = n; }
#endif

    bool tokenIsValid(size_t n) const {
      assert(n < limit);
      return true;
    }

    // Access methods
    return_type operator[](size_t n) {
      //std::cerr << "smoc_multiplex_vfifo_outlet<T,A>::AccessImpl::operator[](size_t) BEGIN" << std::endl;
      assert(n < limit);
      MultiplexChannel &chan = getChan();
      
      size_t rindex;
      
      //std::cerr << "XXX " << getChanIfImpl().fifoId << std::endl;
      
      for (rindex = chan.rIndex();
           n >= 1 || A::get(chan.storage[rindex].get()) != getChanIfImpl().fifoId;
           rindex = rindex < chan.fSize() - 1 ? rindex + 1 : 0)
        if (A::get(chan.storage[rindex].get()) == getChanIfImpl().fifoId)
          --n;
      assert(A::get(chan.storage[rindex].get()) == getChanIfImpl().fifoId);
      //std::cerr << "smoc_multiplex_vfifo_outlet<T,A>::AccessImpl::operator[](size_t) END" << std::endl;
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
  AccessImpl              accessImpl;
public:
  /// @brief Constructor
  smoc_multiplex_vfifo_outlet(const PMultiplexChannel &chan, FifoId fifoId)
    : chan(chan), fifoId(fifoId), countAvailable(0) {
    chan->registerVOutlet(std::make_pair(
      fifoId,
      std::bind1st(std::mem_fun(&this_type::latencyExpired), this)
    ));
  }

  ~smoc_multiplex_vfifo_outlet() {
    chan->deregisterVOutlet(fifoId);
  }
protected:
  /// @brief See smoc_port_in_base_if
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

  /// @brief See smoc_port_in_base_if
  smoc_event &dataAvailableEvent(size_t n)
    { return emmAvailable.getEvent(countAvailable, n); }

  /// @brief See smoc_port_in_base_if
  size_t numAvailable() const
    { return countAvailable; }

  /// @brief See smoc_port_in_base_if
  size_t inTokenId() const
    { return -1; }

  /// @brief See smoc_port_in_if
  AccessImpl *getReadPortAccess()
    { return &accessImpl; }

  void latencyExpired(size_t n) {
    countAvailable += n;
    emmAvailable.increasedCount(countAvailable);
  }
};

template<class T, class A>
class smoc_multiplex_vfifo_entry
: public smoc_port_out_if<T,smoc_1d_port_access_if> {
  typedef smoc_multiplex_vfifo_entry<T,A> this_type;
  // Ugh need this friend decl for the AccessImpl friend decl in
  // smoc_multiplex_fifo_chan
  friend class smoc_multiplex_fifo_chan<T,A>;
private:
  typedef smoc_multiplex_fifo_chan<T,A>       MultiplexChannel;
  typedef boost::shared_ptr<MultiplexChannel> PMultiplexChannel;
  typedef typename MultiplexChannel::FifoId   FifoId;

  class AccessImpl: public this_type::access_type {
    typedef AccessImpl  this_type;
  public:
    typedef smoc_multiplex_vfifo_entry<T,A> ChanIfImpl;
    typedef typename this_type::return_type return_type;
  private:
#if defined(SYSTEMOC_ENABLE_DEBUG)
    size_t limit;
#endif
  private:
    ChanIfImpl &getChanIfImpl() {
      //std::cerr << "offsetof(ChanIfImpl, accessImpl): " <<  offsetof(ChanIfImpl, accessImpl) << std::endl;

      ChanIfImpl *retval =
        reinterpret_cast<ChanIfImpl *>(
          reinterpret_cast<char *>(this) -
          offsetof(ChanIfImpl, accessImpl));
      //std::cerr << "this: " << this << ", retval: " << retval << std::endl;
      return *retval;
    }
    MultiplexChannel &getChan()
      { return *getChanIfImpl().chan.get(); }
  public:
    AccessImpl()
#if defined(SYSTEMOC_ENABLE_DEBUG)
      : limit(0)
#endif
      {}

#if defined(SYSTEMOC_ENABLE_DEBUG)
    void setLimit(size_t n)
      { limit = n; }
#endif

    bool tokenIsValid(size_t n) const {
      assert(n < limit);
      return true;
    }

    // Access methods
    return_type operator[](size_t n) {
      //std::cerr << "smoc_multiplex_vfifo_entry<T,A>::AccessImpl::operator[](size_t) BEGIN" << std::endl;
      assert(n < limit);
      MultiplexChannel &chan = getChan();
      size_t windex = chan.wIndex() + n;
      if (windex >= chan.fSize())
        windex -= chan.fSize();
      //std::cerr << "smoc_multiplex_vfifo_entry<T,A>::AccessImpl::operator[](size_t) END" << std::endl;
      return chan.storage[chan.wIndex() + n];
    }

    const return_type operator[](size_t n) const
      { return const_cast<this_type *>(this)->operator[](n); }
  };
private:
  /// @brief The channel implementation
  PMultiplexChannel chan;
  FifoId            fifoId;
  AccessImpl        accessImpl;
public:
  /// @brief Constructor
  smoc_multiplex_vfifo_entry(const PMultiplexChannel &chan, FifoId fifoId)
    : chan(chan), fifoId(fifoId) {}
protected:
  /// @brief See smoc_port_out_base_if
#ifdef SYSTEMOC_ENABLE_VPC
  void commitWrite(size_t n, const smoc_ref_event_p &latEvent)
#else
  void commitWrite(size_t n)
#endif
  {
    size_t windex = chan->wIndex();
    size_t fsize  = chan->fSize();
    
    for (size_t i = 0; i < n; ++i) {
      A::put(chan->storage[windex].get(), fifoId);
      if (++windex >= fsize)
        windex -= fsize;
    }
    
    // This will do a callback to latencyExpired(produce) at the appropriate time
#ifdef SYSTEMOC_ENABLE_VPC
    chan->commitWrite(n, latEvent); 
#else
    chan->commitWrite(n); 
#endif
  }

  /// @brief See smoc_port_out_base_if
  smoc_event &spaceAvailableEvent(size_t n)
    { return chan->spaceAvailableEvent(n); }

  /// @brief See smoc_port_out_base_if
  size_t numFree() const
    { return chan->freeCount(); }

  /// @brief See smoc_port_out_base_if
  size_t outTokenId() const
    { return -1; }

  /// @brief See smoc_port_out_if
  AccessImpl *getWritePortAccess()
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
  class chan_init
  : public SysteMoC::Detail::ConnectProvider<
      chan_init,
      smoc_multiplex_vfifo_chan<T,A> > {
    typedef chan_init this_type;

    friend class SysteMoC::Detail::ConnectProvider<this_type, typename this_type::chan_type>;
    friend class smoc_multiplex_vfifo_chan<T,A>;
  public:
    typedef T                               data_type;
    typedef smoc_multiplex_vfifo_chan<T,A>  chan_type;
  private:
    FifoId                      fifoId;
    PMultiplexChannel           pMultiplexChan;
    smoc_multiplex_vfifo_chan  *dummy;
  protected:
    typedef const T &add_param_ty;
  public:
    chan_init(FifoId fifoId, const PMultiplexChannel &pMultiplexChan)
      : fifoId(fifoId), pMultiplexChan(pMultiplexChan),
        dummy(new chan_type(*this))
      {}

    void add(const add_param_ty x) {
      A::put(x, this->fifoId);
      pMultiplexChan->storage[pMultiplexChan->wIndex()] = x;
      pMultiplexChan->commitWrite(1);
    }
  private:
    chan_type *getChan()
      { return dummy; }
  };
private:
  FifoId            fifoId;
  PMultiplexChannel pMultiplexChan;
public:
  /// @brief Constructor
  smoc_multiplex_vfifo_chan(const chan_init &i)
    : fifoId(i.fifoId), pMultiplexChan(i.pMultiplexChan)
    {}

protected:
  /// @brief See smoc_port_registry
  smoc_port_out_base_if* createEntry()
    { return new entry_type(pMultiplexChan, fifoId); }

  /// @brief See smoc_port_registry
  smoc_port_in_base_if* createOutlet()
    { return new outlet_type(pMultiplexChan, fifoId); }

private:
};

template <typename T, typename A = typename T::ColorAccessor>
class smoc_multiplex_fifo
: public smoc_multiplex_fifo_chan<T,A>::chan_init,
  public SysteMoC::Detail::ConnectProvider<
    smoc_multiplex_fifo<T,A>,
    smoc_multiplex_fifo_chan<T,A> > {
  typedef smoc_multiplex_fifo<T,A> this_type;

  friend class SysteMoC::Detail::ConnectProvider<this_type, typename this_type::chan_type>;
private:
  typedef typename smoc_multiplex_fifo_chan<T,A>::chan_init base_type;
public:
  typedef typename this_type::chan_type chan_type;

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

  this_type &operator <<(typename this_type::add_param_ty x)
    { add(x); return *this; }

  /// Backward compatibility cruft
  this_type &operator <<(smoc_port_out<T> &p)
    { return this->connect(p); }
  this_type &operator <<(smoc_port_in<T> &p)
    { return this->connect(p); }
  template<class IFACE>
  this_type &operator <<(sc_port<IFACE> &p)
    { return this->connect(p); }
private:
  chan_type *getChan()
    { return pMultiplexChan.get(); }
};

#endif // _INCLUDED_SMOC_MULTIPLEX_FIFO_HPP
