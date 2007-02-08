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

#ifndef _INCLUDED_SMOC_FIFO_HPP
#define _INCLUDED_SMOC_FIFO_HPP

#include <cosupport/commondefs.h>

#include <smoc_chan_if.hpp>
#include <smoc_storage.hpp>

#include <systemc.h>
#include <vector>
#include <queue>
#include <map>

#include <hscd_tdsim_TraceLog.hpp>

// #include <iostream>

class smoc_fifo_kind;

namespace smoc_detail {
#ifdef ENABLE_SYSTEMC_VPC
  class LatencyQueue {
    friend class smoc_fifo_kind;
  public:
    typedef LatencyQueue this_type;
  protected:
    class RequestQueue
    : public smoc_event_listener {
    protected:
      typedef std::pair<size_t, smoc_ref_event_p> Entry;
      typedef std::queue<Entry>                   Queue;
    protected:
      Queue queue;

      smoc_event dummy;
    protected:
      LatencyQueue &getTop() {
        // MAGIC BEGINS HERE
        return *reinterpret_cast<LatencyQueue *>
          (reinterpret_cast<char *>(this) + 4711 -
           reinterpret_cast<char *>
            (&reinterpret_cast<LatencyQueue *>(4711)->requestQueue));
      }

      void doSomething(size_t n);

      bool signaled(smoc_event_waiter *_e) {
        size_t n = 0;
        
        assert(*_e);
        assert(!queue.empty());
        assert(_e == &*queue.front().second);
        _e->delListener(this);
        do {
          n += queue.front().first;
          queue.pop(); // pop from front of queue
        } while (!queue.empty() && *queue.front().second);
        doSomething(n);
        if (!queue.empty())
          queue.front().second->addListener(this);
        return false;
      }

      void eventDestroyed(smoc_event_waiter *_e)
        { assert(!"eventDestroyed must never be called !!!"); }
    public:
      void addEntry(size_t n, const smoc_ref_event_p &le) {
        bool queueEmpty = queue.empty();
        
        if (queueEmpty && (!le || *le)) {
          doSomething(n);
        } else {
          queue.push(Entry(n, le)); // insert at back of queue
          if (queueEmpty)
            le->addListener(this);
        }
      }

      virtual ~RequestQueue() {}
    } requestQueue;

    class VisibleQueue
    : public smoc_event_listener {
    protected:
      typedef std::pair<size_t, smoc_ref_event_p> Entry;
      typedef std::queue<Entry>                   Queue;
    protected:
      Queue queue;
    protected:
      LatencyQueue &getTop() {
        // MAGIC BEGINS HERE
        return *reinterpret_cast<LatencyQueue *>
          (reinterpret_cast<char *>(this) + 411 -
           reinterpret_cast<char *>
            (&reinterpret_cast<LatencyQueue *>(411)->visibleQueue));
      }

      void doSomething(size_t n);

      bool signaled(smoc_event_waiter *_e) {
        size_t n = 0;
        
        assert(*_e);
        assert(!queue.empty());
        assert(_e == &*queue.front().second);
        _e->delListener(this);
        do {
          n += queue.front().first;
          queue.pop(); // pop from front of queue
        } while (!queue.empty() && *queue.front().second);
        doSomething(n);
        if (!queue.empty())
          queue.front().second->addListener(this);
        return false;
      }

      void eventDestroyed(smoc_event_waiter *_e)
        { assert(!"eventDestroyed must never be called !!!"); }
    public:
      void addEntry(size_t n, const smoc_ref_event_p &le) {
        bool queueEmpty = queue.empty();
        
        if (queueEmpty && (!le || *le)) {
          doSomething(n);
        } else {
          queue.push(Entry(n, le)); // insert at back of queue
          if (queueEmpty)
            le->addListener(this);
        }
      }

      virtual ~VisibleQueue() {}
    } visibleQueue;

    smoc_fifo_kind *fifo;
  protected:
    LatencyQueue(smoc_fifo_kind *fifo)
      : fifo(fifo) {}

    void addEntry(size_t n, const smoc_ref_event_p &le)
      { requestQueue.addEntry(n, le); }
  };
#endif // ENABLE_SYSTEMC_VPC
};

/// Base class of the FIFO implementation.
/// The FIFO consists of a ring buffer of size fsize.
/// However due to the ambiguity of rindex == windex
/// indicating either an empty or a full FIFO, which
/// is resolved to rindex == windex indicating an empty
/// FIFO, one ring buffer entry is lost. Therefore the
/// ringe buffer must be one entry greater than the
/// actual FIFO size.
/// Furthermore the storage of the ring buffer is allocated
/// in a derived class the base class only manages the
/// read, write, and visible pointers.
class smoc_fifo_kind
: public smoc_nonconflicting_chan {
#ifdef ENABLE_SYSTEMC_VPC
  friend class smoc_detail::LatencyQueue::VisibleQueue;
#endif // ENABLE_SYSTEMC_VPC
public:
  typedef smoc_fifo_kind  this_type;

  class chan_init {
    friend class smoc_fifo_kind;
  private:
    const char *name;
    size_t      n;
  protected:
    chan_init( const char *name, size_t n )
      : name(name), n(n) {}
  };
protected:
  typedef std::map<size_t, smoc_event *>      EventMap;

#ifdef ENABLE_SYSTEMC_VPC
  smoc_detail::LatencyQueue   latencyQueue;
#endif

  size_t const fsize;   ///< Ring buffer size == FIFO size + 1
  size_t       rindex;  ///< The FIFO read    ptr
#ifdef ENABLE_SYSTEMC_VPC
  size_t       vindex;  ///< The FIFO visible ptr
#endif
  size_t       windex;  ///< The FIFO write   ptr

  smoc_event   eventWrite;
  EventMap     eventMapAvailable;
  smoc_event   eventRead;
  EventMap     eventMapFree;

  size_t usedStorage() const {
    size_t used =
#ifdef ENABLE_SYSTEMC_VPC
      vindex - rindex;
#else
      windex - rindex;
#endif
    
    if ( used > fsize )
      used += fsize;
    return used;
    // less lines of code but slightly slower
    //return (used + fsize) % fsize;
  }

  void usedIncr() {
    size_t used = usedStorage();
    // notify all disabled events for less/equal usedStorage() available tokens
    for (EventMap::const_iterator iter = eventMapAvailable.upper_bound(used);
         iter != eventMapAvailable.begin() && !*(--iter)->second;
         )
      iter->second->notify();
    eventWrite.notify();
  }

  void usedDecr() {
    size_t used = usedStorage();
    // reset all enabled events for more then usedStorage() available tokens
    for (EventMap::const_iterator iter = eventMapAvailable.upper_bound(used);
         iter != eventMapAvailable.end() && *iter->second;
         ++iter)
      iter->second->reset();
  }

  size_t unusedStorage() const {
    size_t unused = rindex - windex - 1;

    if ( unused > fsize )
      unused += fsize;
    return unused;
  }

  void unusedIncr() {
    size_t unused = unusedStorage();
    // notify all disabled events for less/equal unusedStorage() free space
    for (EventMap::const_iterator iter = eventMapFree.upper_bound(unused);
         iter != eventMapFree.begin() && !*(--iter)->second;
         )
      iter->second->notify();
    eventRead.notify();
  }

  void unusedDecr() {
    size_t unused = unusedStorage();
    // reset all enabled events for more then unusedStorage() free space
    for (EventMap::const_iterator iter = eventMapFree.upper_bound(unused);
         iter != eventMapFree.end() && *iter->second;
         ++iter)
      iter->second->reset();
  }

  void rpp(size_t n) {
    /*if ( rindex + n >= fsize )
      rindex = rindex + n - fsize;
    else
      rindex = rindex + n;*/
    rindex = (rindex + n) % fsize;
    
    usedDecr(); unusedIncr();
  }

#ifdef ENABLE_SYSTEMC_VPC
  void wpp(size_t n, const smoc_ref_event_p &le)
#else
  void wpp(size_t n)
#endif
  {
    /*if ( windex + n >= fsize )
      windex = windex + n - fsize;
    else
      windex = windex + n;*/
    windex = (windex + n) % fsize;
    
    unusedDecr();
#ifdef ENABLE_SYSTEMC_VPC
    latencyQueue.addEntry(n, le);
#else
    usedIncr();
#endif
  }

#ifdef ENABLE_SYSTEMC_VPC
  void incrVisible(size_t n) {
# ifndef NDEBUG
    size_t oldUsed = usedStorage();
# endif
    /*if ( vindex + n >= fsize )
      vindex = vindex + n - fsize;
    else
      vindex = vindex + n;*/
    vindex = (vindex + n) % fsize;
    // PARANOIA: rindex <= visible <= windex in modulo fsize arith
    assert(vindex < fsize &&
      (windex >= rindex && (vindex >= rindex && vindex <= windex) ||
       windex <  rindex && (vindex >= rindex || vindex <= windex)));
# ifndef NDEBUG
    // PARANOIA: usedStorage() must increase
    assert(usedStorage() >= oldUsed);
# endif
    usedIncr();
  }
#endif

  smoc_event &getEventAvailable(size_t n) {
    if (n != MAX_TYPE(size_t)) {
      EventMap::iterator iter = eventMapAvailable.find(n);
      if (iter == eventMapAvailable.end()) {
        iter = eventMapAvailable.insert(EventMap::value_type(n, new smoc_event())).first;
        if (usedStorage() >= n)
          iter->second->notify();
      }
      return *iter->second;
    } else {
      return eventWrite;
    }
  }

  smoc_event &getEventFree(size_t n) {
    if (n != MAX_TYPE(size_t)) {
      EventMap::iterator iter = eventMapFree.find(n);
      if (iter == eventMapFree.end()) {
        iter = eventMapFree.insert(EventMap::value_type(n, new smoc_event())).first;
        if (unusedStorage() >= n)
          iter->second->notify();
      }
      return *iter->second;
    } else {
      return eventRead;
    }
  }

  void channelAttributes(smoc_modes::PGWriter &pgw) const {
    pgw << "<attribute type=\"size\" value=\"" << (fsize - 1) << "\"/>" << std::endl;
  }

  virtual
  void channelContents(smoc_modes::PGWriter &pgw) const = 0;

  // constructors
  smoc_fifo_kind( const chan_init &i )
    : smoc_nonconflicting_chan(
        i.name != NULL ? i.name : sc_gen_unique_name( "smoc_fifo" ) ),
#ifdef ENABLE_SYSTEMC_VPC
      latencyQueue(this), 
#endif
      fsize(i.n+1),
      rindex(0),
#ifdef ENABLE_SYSTEMC_VPC
      vindex(0), 
#endif
      windex(0)
  {
    // for lazy % overflow protection fsize must be less than half the datatype
    //  size
    assert(fsize < (MAX_TYPE(size_t) >> 1));
  }
private:
  static const char* const kind_string;
  
  virtual const char* kind() const {
    return kind_string;
  }
  
  // disabled
  smoc_fifo_kind( const this_type & );
  this_type& operator = ( const this_type & );
};

template <typename T>
class smoc_fifo_storage
: public smoc_chan_if<smoc_fifo_kind,
		      T,
		      smoc_channel_access,
		      smoc_channel_access> {
public:
  typedef T                                   data_type;
  typedef smoc_fifo_storage<data_type>        this_type;
  typedef typename this_type::access_out_type ring_out_type;
  typedef typename this_type::access_in_type  ring_in_type;
  typedef smoc_storage<data_type>	      storage_type;
  typedef smoc_ring_access<
    typename ring_in_type::storage_type,
    typename ring_in_type::return_type>       ring_access_in_type;
  typedef smoc_ring_access<
    typename ring_out_type::storage_type,
    typename ring_out_type::return_type>      ring_access_out_type;
  
  class chan_init
    : public smoc_fifo_kind::chan_init {
    friend class smoc_fifo_storage<T>;
  private:
    std::vector<T>  marking;
  protected:
    typedef const T add_param_ty;
  public:
    void add( add_param_ty x ) {
      marking.push_back(x);
    }
  protected:
    chan_init( const char *name, size_t n )
      : smoc_fifo_kind::chan_init(name, n) {}
  };
private:
  storage_type *storage;
protected:
  smoc_fifo_storage( const chan_init &i )
//  : smoc_chan_nonconflicting_if<smoc_fifo_kind, T>(i),
    : smoc_chan_if<smoc_fifo_kind,
		   T,
		   smoc_channel_access,
		   smoc_channel_access>(i),
      storage(new storage_type[this->fsize])
  {
    assert(this->fsize > i.marking.size());
    for(size_t j = 0; j < i.marking.size(); ++j) {
      storage[j].put(i.marking[j]);
    }
    this->windex = i.marking.size();
#ifdef ENABLE_SYSTEMC_VPC
    this->vindex = this->windex;
#endif
  }

  ring_in_type * accessSetupIn() {
    ring_access_in_type *r = new ring_access_in_type();
    r->storage     = storage;
    r->storageSize = this->fsize;
    r->offset      = &this->rindex;
    return r;
  }
  ring_out_type * accessSetupOut() {
    ring_access_out_type *r = new ring_access_out_type();
    r->storage     = storage;
    r->storageSize = this->fsize;
    r->offset      = &this->windex; 
    return r;
  }

  void channelContents(smoc_modes::PGWriter &pgw) const {
    pgw << "<fifo tokenType=\"" << typeid(data_type).name() << "\">" << std::endl;
    {
      //*************************INITIAL TOKENS, ETC...***************************
      pgw.indentUp();
      for ( size_t n = 0; n < this->usedStorage(); ++n )
        pgw << "<token value=\"" << storage[n].get() << "\"/>" << std::endl;
      pgw.indentDown();
    }
    pgw << "</fifo>" << std::endl;
  }

  ~smoc_fifo_storage() { delete[] storage; }
};

template <>
class smoc_fifo_storage<void>
//: public smoc_chan_nonconflicting_if<smoc_fifo_kind, void> {
: public smoc_chan_if<smoc_fifo_kind,
		      void,
		      smoc_channel_access,
		      smoc_channel_access> {
public:
  typedef void                          data_type;
  typedef smoc_fifo_storage<data_type>  this_type;
  typedef this_type::access_out_type    ring_out_type;
  typedef this_type::access_in_type     ring_in_type;
  typedef smoc_ring_access<
    ring_in_type::storage_type,
    ring_in_type::return_type>       ring_access_in_type;
  typedef smoc_ring_access<
    ring_out_type::storage_type,
    ring_out_type::return_type>      ring_access_out_type;
  
  class chan_init
    : public smoc_fifo_kind::chan_init {
    friend class smoc_fifo_storage<void>;
  private:
    size_t          marking;
  protected:
    typedef size_t  add_param_ty;
  public:
    void add( add_param_ty x ) {
      marking += x;
    }
  protected:
    chan_init( const char *name, size_t n )
      : smoc_fifo_kind::chan_init(name, n),
        marking(0) {}
  };
protected:
  smoc_fifo_storage( const chan_init &i )
//  : smoc_chan_nonconflicting_if<smoc_fifo_kind, void>(i) {
    : smoc_chan_if<smoc_fifo_kind,
		   void,
		   smoc_channel_access,
		   smoc_channel_access>(i) {
    assert( fsize > i.marking );
    windex = i.marking;
#ifdef ENABLE_SYSTEMC_VPC
    vindex = windex;
#endif
  }
 
  ring_in_type  * accessSetupIn()  {
    ring_access_in_type *r = new ring_access_in_type();
    return r;
  }
  ring_out_type * accessSetupOut() {
    ring_access_out_type *r = new ring_access_out_type();
    return r;
  }

  void channelContents(smoc_modes::PGWriter &pgw) const {
    pgw << "<fifo tokenType=\"" << typeid(data_type).name() << "\">" << std::endl;
    {
      //*************************INITIAL TOKENS, ETC...***************************
      pgw.indentUp();
      for ( size_t n = 0; n < this->usedStorage(); ++n )
        pgw << "<token value=\"bot\"/>" << std::endl;
      pgw.indentDown();
    }
    pgw << "</fifo>" << std::endl;
  }
};

template <typename T>
class smoc_fifo_type
  : public smoc_fifo_storage<T> {
public:
  typedef T						      data_type;
  typedef smoc_fifo_type<data_type>			      this_type;
  
  typedef typename smoc_storage_in<data_type>::storage_type   storage_in_type;
  typedef typename smoc_storage_in<data_type>::return_type    return_in_type;
  
  typedef typename smoc_storage_out<data_type>::storage_type  storage_out_type;
  typedef typename smoc_storage_out<data_type>::return_type   return_out_type;
protected:
  
#ifdef ENABLE_SYSTEMC_VPC
  void commExecIn(size_t consume, const smoc_ref_event_p &le)
#else
  void commExecIn(size_t consume)
#endif
  {
#ifdef SYSTEMOC_TRACE
    TraceLog.traceCommExecIn(consume, this->name());
#endif
    this->rpp(consume);
//  this->read_event.notify(); 
//  if (this->usedStorage() < 1)
//    this->write_event.reset();
  }
  
#ifdef ENABLE_SYSTEMC_VPC
  void commExecOut(size_t produce, const smoc_ref_event_p &le)
#else
  void commExecOut(size_t produce)
#endif
  {
#ifdef SYSTEMOC_TRACE
    TraceLog.traceCommExecOut(produce, this->name());
#endif
#ifdef ENABLE_SYSTEMC_VPC
    this->wpp(produce, le);
#else
    this->wpp(produce);
#endif
//  this->write_event.notify();
//  if (this->unusedStorage() < 1)
//    this->read_event.reset();
  }
public:
  // constructors
  smoc_fifo_type( const typename smoc_fifo_storage<T>::chan_init &i )
    : smoc_fifo_storage<T>(i) {
//  if (this->usedStorage() >= 1)
//    this->write_event.notify();
//  if (this->unusedStorage() >= 1)
//    this->read_event.notify();
  }

  // bounce functions
  size_t committedOutCount() const
    { return this->usedStorage(); }
  size_t committedInCount() const
    { return this->unusedStorage(); }
  smoc_event &blockEventOut(size_t n)
    { return this->getEventAvailable(n); }
  smoc_event &blockEventIn(size_t n)
    { return this->getEventFree(n); }
private:
    void reset(){};
};

template <typename T>
class smoc_fifo
  : public smoc_fifo_storage<T>::chan_init {
public:
  typedef T                   data_type;
  typedef smoc_fifo<T>        this_type;
  typedef smoc_fifo_type<T>   chan_type;
  
  this_type &operator <<( typename smoc_fifo_storage<T>::chan_init::add_param_ty x ) {
    add(x); return *this;
  }
  
  smoc_fifo( size_t n = 1 )
    : smoc_fifo_storage<T>::chan_init(NULL,n) {}
  explicit smoc_fifo( const char *name, size_t n = 1)
    : smoc_fifo_storage<T>::chan_init(name,n) {}
private:
    void reset(){};
};

#endif // _INCLUDED_SMOC_FIFO_HPP
