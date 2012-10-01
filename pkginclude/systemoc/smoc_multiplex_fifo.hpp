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

#include <algorithm>
#include <utility>
#include <vector>
#include <queue>
#include <map>

#include <CoSupport/commondefs.h>

#include <systemc>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

#include <systemoc/smoc_config.h>

#ifdef SYSTEMOC_ENABLE_VPC
# include <vpc.hpp>
#endif //SYSTEMOC_ENABLE_VPC

#include "detail/smoc_root_chan.hpp"
#include "detail/smoc_chan_if.hpp"
#include "detail/smoc_storage.hpp"
#include "smoc_chan_adapter.hpp"
#include "smoc_fifo.hpp"
#include "detail/smoc_ring_access.hpp"
#include <smoc/detail/ConnectProvider.hpp>
#include <smoc/detail/EventMapManager.hpp>
#ifdef SYSTEMOC_ENABLE_VPC
# include <smoc/detail/LatencyQueue.hpp>
# include <smoc/detail/DIIQueue.hpp>
# include <smoc/detail/QueueFRVWPtr.hpp>
#endif
#include <smoc/detail/QueueRWPtr.hpp>

#include <smoc/detail/DumpingInterfaces.hpp>

class smoc_multiplex_vfifo_chan_base;

template <class T, class A> class smoc_multiplex_fifo_entry;
template <class T, class A> class smoc_multiplex_fifo_outlet;
template <class T, class A> class smoc_multiplex_vfifo_entry;
template <class T, class A> class smoc_multiplex_vfifo_outlet;

class smoc_multiplex_fifo_chan_base
: private boost::noncopyable,
  public smoc_root_chan,
#ifdef SYSTEMOC_ENABLE_VPC
  public smoc::Detail::QueueFRVWPtr
#else
  public smoc::Detail::QueueRWPtr
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
  typedef std::map<FifoId, smoc::Detail::PortInBaseIf  *> VOutletMap;
  typedef std::map<FifoId, smoc::Detail::PortOutBaseIf *> VEntryMap;
protected:
  VOutletMap    vOutlets;
  VEntryMap     vEntries;
  const size_t  fifoOutOfOrder; // == 0 => no out of order access only one element visible

  // This are the EventMapManager for the plain fifo access operations
  smoc::Detail::EventMapManager emmFree;      // for smoc_multiplex_fifo_entry
  smoc::Detail::EventMapManager emmAvailable; // for smoc_multiplex_fifo_outlet
protected:
  smoc_multiplex_fifo_chan_base(const chan_init &i);

  void registerVOutlet(const VOutletMap::value_type &entry);
  void deregisterVOutlet(FifoId fifoId);

  void registerVEntry(const VEntryMap::value_type &entry);
  void deregisterVEntry(FifoId fifoId);

  /// @brief See PortInBaseIf
  smoc::smoc_event &dataAvailableEvent(size_t n)
    { return emmAvailable.getEvent(n); }

  smoc::smoc_event &spaceAvailableEvent(size_t n)
    { return emmFree.getEvent(n); }

  void doReset() {
    emmFree.reset();
    emmAvailable.reset();

    emmFree.increasedCount(freeCount());
    if(vOutlets.empty()) {
      // if there are outlets, doReset of derived channel will take care
      emmAvailable.increasedCount(visibleCount());
    }
    
    smoc_root_chan::doReset();
  }

  /// @brief See PortInBaseIf
  size_t inTokenId() const
    { return static_cast<size_t>(-1); }
public:
#ifdef SYSTEMOC_ENABLE_SGX
  // FIXME: This should be protected for the SysteMoC user but accessible
  // for SysteMoC visitors
  virtual void dumpInitialTokens(smoc::Detail::IfDumpingInitialTokens *it) = 0;

  /// @brief Returns virtual entries
  const VEntryMap &getVEntries() const
    { return vEntries; }

  /// @brief Returns virtual outlets
  const VOutletMap &getVOutlets() const
    { return vOutlets; }
#endif // SYSTEMOC_ENABLE_SGX
};

template<class T, class A>
class smoc_multiplex_fifo;
template<class T, class A>
class smoc_multiplex_vfifo_chan;

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
  friend class smoc_multiplex_vfifo_chan<T,A>::chan_init; // access to storage
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
  smoc::Detail::LatencyQueue  latencyQueue;
  smoc::Detail::DIIQueue      diiQueue;
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

  void pushToVOutlets(typename this_type::MG xindex, size_t oindex) {
    for (; xindex != oindex; ++xindex) {
      FifoId fId = A::get(this->storage[xindex.getValue()].get());
      typename this_type::VOutletMap::iterator fIter = this->vOutlets.find(fId);
      assert(fIter != this->vOutlets.end());
      smoc_multiplex_vfifo_outlet<T, A> *voutlet =
        static_cast<smoc_multiplex_vfifo_outlet<T, A> *>(fIter->second);
      voutlet->storage[voutlet->wIndex()].put(this->storage[xindex.getValue()].get());
      voutlet->commitWrite(1);
    }
  }

  void doReset() {
    // writes initial tokens and resets queue etc.
    typedef typename this_type::MG    MG;
    typedef typename this_type::MG::M M;
    
    base_type::doReset();
    if (!this->vOutlets.empty()) {
      // if there are no outlets, base channel class will take care
      for (typename VOutletMap::const_iterator o = this->vOutlets.begin();
          o != this->vOutlets.end();
          ++o)
        static_cast<smoc_multiplex_vfifo_outlet<T, A> *>(o->second)->reset();
      pushToVOutlets(
        MG(this->rIndex(), M(this->fSize())),
        (MG(this->rIndex(), M(this->fSize())) +
          std::min(this->fifoOutOfOrder + 1, this->visibleCount())).getValue());
    }
  }

#ifdef SYSTEMOC_ENABLE_VPC
  void commitWrite(size_t n, smoc::Detail::VpcInterface vpcIf)
#else
  void commitWrite(size_t n)
#endif
  {
    this->wpp(n);
    this->emmFree.decreasedCount(this->freeCount());
#ifdef SYSTEMOC_ENABLE_VPC
    // Delayed call of latencyExpired(n);
    latencyQueue.addEntry(n, vpcIf.getTaskLatEvent(), vpcIf);
#else
    latencyExpired(n);
#endif
  }

  void latencyExpired(size_t n) {
//  std::cerr << "smoc_multiplex_fifo_chan_base::latencyExpired(" << n << ") [BEGIN]" << std::endl;
//  std::cerr << "fifoOutOfOrder == " << fifoOutOfOrder << std::endl;
//  std::cerr << "freeCount():    " << freeCount() << std::endl;
//  std::cerr << "usedCount():    " << usedCount() << std::endl;
//  std::cerr << "visibleCount(): " << visibleCount() << std::endl;
    typedef typename this_type::MG    MG;
    typedef typename this_type::MG::M M;
    
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
      
      pushToVOutlets(
        // Calculate xindex to point to the first token inside or one outside the
        // OOO area for which the latency has expired.
        MG(this->rIndex(), M(this->fSize())) +
          std::min(this->fifoOutOfOrder + 1, vcount - n),
        // Calculate oindex to point to the first token either outside
        // the OOO area or outside the visible area.
        (MG(this->rIndex(), M(this->fSize())) +
          std::min(this->fifoOutOfOrder + 1, vcount)).getValue());
    } else {
      this->emmAvailable.increasedCount(vcount);
    }
//  std::cerr << "smoc_multiplex_fifo_chan_base::latencyExpired(" << n << ") [END]" << std::endl;
//  std::cerr << "freeCount():    " << freeCount() << std::endl;
//  std::cerr << "usedCount():    " << usedCount() << std::endl;
//  std::cerr << "visibleCount(): " << visibleCount() << std::endl;
  }

#ifdef SYSTEMOC_ENABLE_VPC
  void consume(FifoId from, size_t n, smoc::Detail::VpcInterface vpcIf)
#else
  void consume(FifoId from, size_t n)
#endif
  {
//  std::cerr << "smoc_multiplex_fifo_chan_base::consume(" << from << ", " << n << ") [BEGIN]" << std::endl;
//  std::cerr << "fifoOutOfOrder == " << fifoOutOfOrder << std::endl;
//  std::cerr << "freeCount():    " << freeCount() << std::endl;
//  std::cerr << "usedCount():    " << usedCount() << std::endl;
//  std::cerr << "visibleCount(): " << visibleCount() << std::endl;
    
    typedef typename this_type::MG    MG;
    typedef typename this_type::MG::M M;
    
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
      MG dindex(this->rIndex(), M(this->fSize()));
      
      // Find n'th fifoId == from element in storage
      for (size_t mc = n; mc > 1; ++dindex) {
        // rindex <= dindex < vindex in modulo fsize arith
        assert(dindex.between(this->rIndex(), MG(this->vIndex(), M(this->fSize())) - 1));
        if (A::get(this->storage[dindex.getValue()].get()) == from)
          --mc;
      }
      for (; A::get(this->storage[dindex.getValue()].get()) != from; ++dindex) {
        // rindex <= dindex < vindex in modulo fsize arith
        assert(dindex.between(this->rIndex(), MG(this->vIndex(), M(this->fSize())) - 1));
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
    this->commitRead(n, vpcIf);
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
     *  F: The free space area of size (findex - windex - 1) % fsize
     *  V: The visible token area of size (vindex - rindex) % fsize
     *  P: The token which are still in the pipeline (latency not expired)
     *
     *  O: The OOO area of size fifoOutOfOrder + 1
     *  C: The tokens which have just been consumed
     *  X: The tokens which have just entered the OOO area
     *
     */
    
    size_t vcount = this->visibleCount();
    pushToVOutlets(
      // Calculate xindex to point to the first token for entering the OOO area.
      MG(this->rIndex(), M(this->fSize())) +
        std::min(this->fifoOutOfOrder + 1 - n, vcount),
      // Calculate oindex to point to the first token either outside
      // the OOO area or outside the visible area.
      (MG(this->rIndex(), M(this->fSize())) +
        std::min(this->fifoOutOfOrder + 1, vcount)).getValue());
    
//  std::cerr << "smoc_multiplex_fifo_chan_base::consume(" << from << ", " << n << ") [END]" << std::endl;
//  std::cerr << "freeCount():    " << freeCount() << std::endl;
//  std::cerr << "usedCount():    " << usedCount() << std::endl;
//  std::cerr << "visibleCount(): " << visibleCount() << std::endl;
  }

#ifdef SYSTEMOC_ENABLE_VPC
  void commitRead(size_t n, smoc::Detail::VpcInterface vpcIf)
#else
  void commitRead(size_t n)
#endif
  {
    this->rpp(n);
    this->emmAvailable.decreasedCount(this->visibleCount());
#ifdef SYSTEMOC_ENABLE_VPC
    // Delayed call of diiExpired(n);
    diiQueue.addEntry(n, vpcIf.getTaskDiiEvent(), vpcIf);
#else
    diiExpired(n);
#endif
  }

  void diiExpired(size_t n) {
//  std::cerr << "smoc_multiplex_fifo_chan_base::diiExpired(" << n << ") [BEGIN]" << std::endl;
//  std::cerr << "fifoOutOfOrder == " << fifoOutOfOrder << std::endl;
//  std::cerr << "freeCount():    " << freeCount() << std::endl;
//  std::cerr << "usedCount():    " << usedCount() << std::endl;
//  std::cerr << "visibleCount(): " << visibleCount() << std::endl;

    this->fpp(n);
    this->emmFree.increasedCount(this->freeCount());

//  std::cerr << "smoc_multiplex_fifo_chan_base::diiExpired(" << n << ") [END]" << std::endl;
//  std::cerr << "freeCount():    " << freeCount() << std::endl;
//  std::cerr << "usedCount():    " << usedCount() << std::endl;
//  std::cerr << "visibleCount(): " << visibleCount() << std::endl;
  }
protected:
  /// @brief See smoc_port_registry
  smoc::Detail::PortOutBaseIf *createEntry()
    { return new entry_type(*this); }

  /// @brief See smoc_port_registry
  smoc::Detail::PortInBaseIf *createOutlet()
    { return new outlet_type(*this); }
};

/**
 * This class implements the channel out interface
 */
template<class T, class A>
class smoc_multiplex_fifo_entry
: public smoc_port_out_if<T> {
  typedef smoc_multiplex_fifo_entry<T,A> this_type;
private:
  /// @brief The channel implementation
  smoc_multiplex_fifo_chan<T,A> &chan;
public:
  /// @brief Constructor
  smoc_multiplex_fifo_entry(smoc_multiplex_fifo_chan<T,A> &chan)
    : chan(chan) {}
protected:
  /// @brief See PortOutBaseIf
#ifdef SYSTEMOC_ENABLE_VPC
  void commitWrite(size_t n, smoc::Detail::VpcInterface vpcIf)
    { return chan.commitWrite(n, vpcIf); }
#else
  void commitWrite(size_t n)
    { return chan.commitWrite(n); }
#endif

  /// @brief See PortOutBaseIf
  smoc::smoc_event &spaceAvailableEvent(size_t n)
    { return chan.spaceAvailableEvent(n); }
 
  /// @brief See PortOutBaseIf
  size_t numFree() const
    { return chan.freeCount(); }

  std::string getChannelName() const
    { return chan.name();}
 
  /// @brief See PortOutBaseIf
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
: public smoc_port_in_if<T> {
  typedef smoc_multiplex_fifo_outlet<T,A> this_type;
private:
  /// @brief The channel implementation
  smoc_multiplex_fifo_chan<T,A> &chan;
public:
  /// @brief Constructor
  smoc_multiplex_fifo_outlet(smoc_multiplex_fifo_chan<T,A> &chan)
    : chan(chan) {}
protected:
  /// @brief See PortInBaseIf
#ifdef SYSTEMOC_ENABLE_VPC
  void commitRead(size_t n, smoc::Detail::VpcInterface vpcIf)
    { return chan.commitRead(n, vpcIf); }
#else
  void commitRead(size_t n)
    { return chan.commitRead(n); }
#endif

  /// @brief See PortInBaseIf
  smoc::smoc_event &dataAvailableEvent(size_t n)
    { return chan.dataAvailableEvent(n); }

  /// @brief See PortInBaseIf
  size_t numAvailable() const
    { return chan.visibleCount(); }

  std::string getChannelName() const
    { return chan.name();}

  /// @brief See PortInBaseIf
  size_t inTokenId() const
    { return static_cast<size_t>(-1); }

  /// @brief See smoc_port_in_if
  typename this_type::access_type *getReadPortAccess()
    { return chan.getReadPortAccess(); }
};

template<class T, class A>
class smoc_multiplex_vfifo_outlet
: public smoc_port_in_if<T>,
  public smoc::Detail::QueueWithStorage<T, smoc::Detail::QueueRWPtr>,
  public smoc_ring_access<
    typename smoc_storage_in<T>::storage_type,
    typename smoc_storage_in<T>::return_type
  >
{
  typedef smoc_multiplex_vfifo_outlet<T,A> this_type;

  friend class smoc_multiplex_fifo_chan<T,A>; // for commitWrite and reset
public:
  typedef smoc_multiplex_fifo_chan<T,A>     MultiplexChannel;
  typedef typename MultiplexChannel::FifoId FifoId;
private:
  /// @brief The channel implementation
  MultiplexChannel             *chan;
  FifoId                        fifoId;
  smoc::Detail::EventMapManager emmAvailable;
public:
  /// @brief Constructor
  smoc_multiplex_vfifo_outlet(MultiplexChannel *chan, FifoId fifoId)
    : smoc::Detail::QueueWithStorage<T, smoc::Detail::QueueRWPtr>(chan->fifoOutOfOrder+1),
      smoc_ring_access<
        typename smoc_storage_in<T>::storage_type,
        typename smoc_storage_in<T>::return_type
      >(this->storage, this->fSize(), &this->rIndex()),
      chan(chan), fifoId(fifoId)
  {
    chan->registerVOutlet
      (std::make_pair(fifoId,this));
  }

  ~smoc_multiplex_vfifo_outlet() {
    chan->deregisterVOutlet(fifoId);
  }
protected:
  /// @brief See PortInBaseIf
#ifdef SYSTEMOC_ENABLE_VPC
  void commitRead(size_t n, smoc::Detail::VpcInterface vpcIf)
#else
  void commitRead(size_t n)
#endif
  {
    this->rpp(n);
#ifdef SYSTEMOC_ENABLE_VPC
    chan->consume(fifoId, n, vpcIf);
#else
    chan->consume(fifoId, n);
#endif
    emmAvailable.decreasedCount(this->visibleCount());
  }

  /// @brief See PortInBaseIf
  smoc::smoc_event &dataAvailableEvent(size_t n)
    { return emmAvailable.getEvent(this->visibleCount(), n); }

  /// @brief See PortInBaseIf
  size_t numAvailable() const
    { return this->visibleCount(); }

  /// @brief See PortInBaseIf
  std::string getChannelName() const
    { return chan->name();}

  /// @brief See PortInBaseIf
  size_t inTokenId() const
    { return static_cast<size_t>(-1); }

  /// @brief See smoc_port_in_if
  typename this_type::access_type *getReadPortAccess()
    { return this; }
private:
  /// @brief Used by various methods from smoc_multiplex_fifo_chan<T,A>
  void commitWrite(size_t n) {
    this->wpp(n);
    emmAvailable.increasedCount(this->visibleCount());
  }
  /// @brief Used by smoc_multiplex_fifo_chan<T,A>::doReset()
  void reset() {
    this->resetQueue();
    emmAvailable.reset();
  }
};

template<class T, class A>
class smoc_multiplex_vfifo_entry
: public smoc_port_out_if<T>,
  public smoc_ring_access<
    typename smoc_storage_out<T>::storage_type,
    typename smoc_storage_out<T>::return_type
  >
{
  typedef smoc_multiplex_vfifo_entry<T,A> this_type;
private:
  typedef smoc_multiplex_fifo_chan<T,A>     MultiplexChannel;
  typedef typename MultiplexChannel::FifoId FifoId;
private:
  /// @brief The channel implementation
  MultiplexChannel *chan;
  FifoId            fifoId;
public:
  /// @brief Constructor
  smoc_multiplex_vfifo_entry(MultiplexChannel *chan, FifoId fifoId)
    : smoc_ring_access<
        typename smoc_storage_out<T>::storage_type,
        typename smoc_storage_out<T>::return_type
      >(chan->storage, chan->fSize(), &chan->wIndex()),
      chan(chan), fifoId(fifoId)
  {
    chan->registerVEntry
      (std::make_pair(fifoId, this));
  }

  ~smoc_multiplex_vfifo_entry() {
    chan->deregisterVEntry(fifoId);
  }
protected:
  /// @brief See PortOutBaseIf
#ifdef SYSTEMOC_ENABLE_VPC
  void commitWrite(size_t n, smoc::Detail::VpcInterface vpcIf)
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
    chan->commitWrite(n, vpcIf); 
#else
    chan->commitWrite(n); 
#endif
  }

  /// @brief See PortOutBaseIf
  smoc::smoc_event &spaceAvailableEvent(size_t n)
    { return chan->spaceAvailableEvent(n); }

  /// @brief See PortOutBaseIf
  size_t numFree() const
    { return chan->freeCount(); }

  std::string getChannelName() const
    { return chan->name();}

  /// @brief See PortOutBaseIf
  size_t outTokenId() const
    { return static_cast<size_t>(-1); }

  /// @brief See smoc_port_out_if
  typename this_type::access_type *getWritePortAccess()
    { return this; }
};

template<class T, class A>
class smoc_multiplex_vfifo;

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

  typedef smoc_multiplex_fifo_chan<T,A>     MultiplexChannel;
  typedef typename MultiplexChannel::FifoId FifoId;

  /// @brief Channel initializer
  class chan_init
  : public smoc::Detail::ConnectProvider<
      smoc_multiplex_vfifo<T,A>,
      smoc_multiplex_vfifo_chan<T,A> >
  {  
  public:
    typedef T                               data_type;
    typedef chan_init                       this_type;
    typedef smoc_multiplex_vfifo_chan<T,A>  chan_type;
#ifdef _MSC_VER
    friend typename this_type::con_type;
#else
    friend class this_type::con_type;
#endif // _MSC_VER
    friend class smoc_multiplex_vfifo_chan<T,A>;
    friend class smoc_multiplex_fifo<T,A>;
  private:
    FifoId                      fifoId;
    MultiplexChannel           *pMultiplexChan;
    smoc_multiplex_vfifo_chan  *dummy;
  protected:
    typedef const T& add_param_ty;
    
    chan_init(FifoId fifoId, MultiplexChannel *pMultiplexChan)
      : fifoId(fifoId), pMultiplexChan(pMultiplexChan),
        dummy(new chan_type(*this))
      {}
  
  public:
    void add(add_param_ty x) {
      pMultiplexChan->initialTokens.push_back(x);
      A::put(pMultiplexChan->initialTokens.back(), this->fifoId); 
    }
  
    this_type &operator <<(add_param_ty x)
      { add(x); return *this; }

    using smoc::Detail::ConnectProvider<
      smoc_multiplex_vfifo<T,A>,
      smoc_multiplex_vfifo_chan<T,A> >::operator<<;

    size_t getFifoId() const
      { return fifoId; }
  private:
    chan_type *getChan()
      { return dummy; }
  };
private:
  FifoId            fifoId;
  MultiplexChannel *pMultiplexChan;
public:
  /// @brief Constructor
  smoc_multiplex_vfifo_chan(const chan_init &i)
    : fifoId(i.fifoId),
      pMultiplexChan(i.pMultiplexChan)
    {}

protected:
  /// @brief See smoc_port_registry
  smoc::Detail::PortOutBaseIf *createEntry()
    { return new entry_type(pMultiplexChan, fifoId); }

  /// @brief See smoc_port_registry
  smoc::Detail::PortInBaseIf  *createOutlet()
    { return new outlet_type(pMultiplexChan, fifoId); }

private:
};

template <typename T, typename A = typename T::ColorAccessor>
class smoc_multiplex_vfifo
: private boost::noncopyable,
  public smoc_multiplex_vfifo_chan<T,A>::chan_init {
public:
  typedef smoc_multiplex_vfifo<T,A>     this_type;
  typedef typename this_type::chan_type chan_type;
  typedef typename chan_type::chan_init base_type;

  smoc_multiplex_vfifo(const base_type &b)
    : base_type(b) {}
};

template <typename T, typename A = typename T::ColorAccessor>
class smoc_multiplex_fifo
: public smoc_multiplex_fifo_chan<T,A>::chan_init,
  public smoc::Detail::ConnectProvider<
    smoc_multiplex_fifo<T,A>,
    smoc_multiplex_fifo_chan<T,A> > {
public:
  typedef smoc_multiplex_fifo<T,A>      this_type;
  typedef typename this_type::chan_type chan_type;
  typedef typename chan_type::chan_init base_type;
  typedef smoc_multiplex_vfifo<T,A>     vfifo_type;

  typedef size_t FifoId;

  //typedef boost::shared_ptr<chan_type> PChannel;
  typedef chan_type* PChannel;
  
#ifdef _MSC_VER
  friend typename this_type::con_type;
#else
  friend class this_type::con_type;
#endif // _MSC_VER
  friend class smoc_reset_net;

private:
  FifoId   fifoIdCount;  // For virtual fifo enumeration
  PChannel chan;
public:
  /// @param n size of the shared fifo memory
  /// @param m out of order access, zero is no out of order
  smoc_multiplex_fifo(size_t n = 1, size_t m = 0)
    : base_type("", n, m), fifoIdCount(0), chan(NULL)
    {}

  smoc_multiplex_fifo(const std::string &name, size_t n = 1, size_t m = 0)
    : base_type(name, n, m), fifoIdCount(0), chan(NULL)
    {}

  typename smoc_multiplex_vfifo_chan<T,A>::chan_init getVirtFifo() {
    getChan();
    return typename smoc_multiplex_vfifo_chan<T,A>::chan_init(
        fifoIdCount++, chan);
  }
  
  smoc_multiplex_fifo(const this_type &x)
    : base_type(x), fifoIdCount(0), chan(NULL)
  {
    if(x.chan)
      assert(!"Can't copy initializer: Channel already created!");
  }

  this_type &operator <<(typename this_type::add_param_ty x) {
    if(chan)
      assert(!"Can't place initial token: Channel already created!");
    add(x);
    return *this;
  }

  using smoc::Detail::ConnectProvider<
    smoc_multiplex_fifo<T,A>,
    smoc_multiplex_fifo_chan<T,A> >::operator<<;

private:
  chan_type *getChan() {
    /*if (!chan)
      chan.reset(new chan_type(*this));
    return chan.get();*/
    if (chan == NULL)
      chan = new chan_type(*this);
    return chan;
  }
  
  // disable
  this_type &operator =(const this_type &);
};

#endif // _INCLUDED_SMOC_MULTIPLEX_FIFO_HPP
