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
  class LatencyQueue
  : public smoc_event_listener {
  public:
    typedef LatencyQueue this_type;
  protected:
    typedef std::pair<size_t, smoc_ref_event_p> Entry;
    typedef std::queue<Entry>                   Queue;

    smoc_fifo_kind *fifo;
    Queue           queue;

    bool signaled(smoc_event_waiter *_e);

    void eventDestroyed(smoc_event_waiter *_e);
  public:
    LatencyQueue(smoc_fifo_kind *fifo)
      : fifo(fifo) {}

    void addEntry(size_t n, const smoc_ref_event_p &le);

    virtual ~LatencyQueue() {}
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
: public smoc_root_chan {
  friend class smoc_detail::LatencyQueue;
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
  }

  void usedIncr(size_t used) {
    assert(used == usedStorage());
    // notify all disabled events for less/equal usedStorage() available tokens
    for (EventMap::const_iterator iter = eventMapAvailable.upper_bound(used);
         iter != eventMapAvailable.begin() && !*(--iter)->second;
         )
      iter->second->notify();
    eventWrite.notify();
  }

  void usedDecr(size_t used) {
    assert(used == usedStorage());
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

  void unusedIncr(size_t unused) {
    assert(unused == unusedStorage());
    // notify all disabled events for less/equal unusedStorage() free space
    for (EventMap::const_iterator iter = eventMapFree.upper_bound(unused);
         iter != eventMapFree.begin() && !*(--iter)->second;
         )
      iter->second->notify();
    eventRead.notify();
  }

  void unusedDecr(size_t unused) {
    assert(unused == unusedStorage());
    // reset all enabled events for more then unusedStorage() free space
    for (EventMap::const_iterator iter = eventMapFree.upper_bound(unused);
         iter != eventMapFree.end() && *iter->second;
         ++iter)
      iter->second->reset();
  }

  void rpp(size_t n) {
    if ( rindex + n >= fsize )
      rindex = rindex + n - fsize;
    else
      rindex = rindex + n;
    
    size_t used   = usedStorage();
    usedDecr(used);
    unusedIncr(fsize - used - 1);
  }

#ifdef ENABLE_SYSTEMC_VPC
  void wpp(size_t n, const smoc_ref_event_p &le)
#else
  void wpp(size_t n)
#endif
  {
    if ( windex + n >= fsize )
      windex = windex + n - fsize;
    else
      windex = windex + n;
    
    size_t unused = unusedStorage();
    unusedDecr(unused);
#ifdef ENABLE_SYSTEMC_VPC
    latencyQueue.addEntry(windex, le);
#else
    usedIncr(fsize - unused - 1);
#endif
  }

#ifdef ENABLE_SYSTEMC_VPC
  void incrVisible(size_t visible) {
    // PARANOIA: rindex <= visible <= windex in modulo fsize arith
    assert(visible < fsize &&
      (windex >= rindex && (visible >= rindex && visible <= windex) ||
       windex <  rindex && (visible >= rindex || visible <= windex)));
# ifndef NDEBUG
    size_t oldUsed = usedStorage();
# endif
    vindex = visible;
    // PARANOIA: usedStorage() must increase
    assert(usedStorage() >= oldUsed);
    usedIncr(usedStorage());
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
    pgw << "<attribute type=\"size\" value=\"" << fsize << "\"/>" << std::endl;
  }

  virtual
  void channelContents(smoc_modes::PGWriter &pgw) const = 0;

  // constructors
  smoc_fifo_kind( const chan_init &i )
    : smoc_root_chan(
        i.name != NULL ? i.name : sc_gen_unique_name( "smoc_fifo" ) ),
#ifdef ENABLE_SYSTEMC_VPC
      latencyQueue(this), 
#endif
      fsize(i.n+1),
      rindex(0),
#ifdef ENABLE_SYSTEMC_VPC
      vindex(0), 
#endif
      windex(0) {
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
  : public smoc_chan_nonconflicting_if<smoc_fifo_kind, T> {
public:
  typedef T                                  data_type;
  typedef smoc_fifo_storage<data_type>       this_type;
  typedef typename this_type::iface_out_type iface_out_type;
  typedef typename this_type::iface_in_type  iface_in_type;
  typedef smoc_storage<data_type>	     storage_type;
  
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
  smoc_fifo_storage( const chan_init &i ) :
    smoc_chan_nonconflicting_if<smoc_fifo_kind, T>(i),
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
  
  storage_type *getStorage() const { return storage; }
  
  void channelContents(smoc_modes::PGWriter &pgw) const {
    for ( size_t n = 0; n < this->usedStorage(); ++n )
      pgw << "<token value=\"" << storage[n].get() << "\"/>" << std::endl;
  }

  ~smoc_fifo_storage() { delete[] storage; }
};

template <>
class smoc_fifo_storage<void>
  : public smoc_chan_nonconflicting_if<smoc_fifo_kind, void> {
public:
  typedef void                               data_type;
  typedef smoc_fifo_storage<data_type>       this_type;
  
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
    : smoc_chan_nonconflicting_if<smoc_fifo_kind, void>(i) {
    assert( fsize > i.marking );
    windex = i.marking;
#ifdef ENABLE_SYSTEMC_VPC
    vindex = windex;
#endif
  }
  
  void *getStorage() const { return NULL; }

  void channelContents(smoc_modes::PGWriter &pgw) const {
    for ( size_t n = 0; n < usedStorage(); ++n )
      pgw << "<token value=\"bot\"/>" << std::endl;
  }
};

template <typename T>
class smoc_fifo_type
  : public smoc_fifo_storage<T> {
public:
  typedef T						      data_type;
  typedef smoc_fifo_type<data_type>			      this_type;
  typedef typename this_type::iface_in_type		      iface_in_type;
  typedef typename this_type::iface_out_type		      iface_out_type;
  
  typedef typename smoc_storage_in<data_type>::storage_type   storage_in_type;
  typedef typename smoc_storage_in<data_type>::return_type    return_in_type;
  typedef smoc_ring_access<storage_in_type, return_in_type>   ring_in_type;
  
  typedef typename smoc_storage_out<data_type>::storage_type  storage_out_type;
  typedef typename smoc_storage_out<data_type>::return_type   return_out_type;
  typedef smoc_ring_access<storage_out_type, return_out_type> ring_out_type;
protected:
//  iface_in_type  *in;
//  iface_out_type *out;
  
  ring_in_type commSetupIn(size_t req) {
    assert( req <= this->usedStorage() );
    return ring_in_type(this->getStorage(),
        this->fsize, this->rindex, req);
  }
  
#ifdef ENABLE_SYSTEMC_VPC
  void commExecIn(const ring_in_type &r, const smoc_ref_event_p &le)
#else
  void commExecIn(const ring_in_type &r)
#endif
  {
#ifdef SYSTEMOC_TRACE
    TraceLog.traceCommExecIn(r.getLimit(), this->name());
#endif
    rpp(r.getLimit());
//  this->read_event.notify(); 
//  if (this->usedStorage() < 1)
//    this->write_event.reset();
  }
  
  ring_out_type commSetupOut(size_t req) {
    assert( req <= this->unusedStorage() );
    return ring_out_type(this->getStorage(),
        this->fsize, this->windex, req);
  }
  
#ifdef ENABLE_SYSTEMC_VPC
  void commExecOut(const ring_out_type &r, const smoc_ref_event_p &le)
#else
  void commExecOut(const ring_out_type &r)
#endif
  {
#ifdef SYSTEMOC_TRACE
    TraceLog.traceCommExecOut(r.getLimit(), this->name());
#endif
#ifdef ENABLE_SYSTEMC_VPC
    wpp(r.getLimit(), le);
#else
    wpp(r.getLimit());
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
};

#endif // _INCLUDED_SMOC_FIFO_HPP
