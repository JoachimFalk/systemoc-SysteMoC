//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
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

#include <CoSupport/commondefs.h>

#include <systemoc/smoc_config.h>

#include "smoc_chan_if.hpp"
#include "smoc_storage.hpp"
#include "LatencyQueue.hpp"

#include <systemc.h>
#include <vector>
#include <map>

#include "hscd_tdsim_TraceLog.hpp"

// FIXME
// -jens- this seems not be an access but an access + storage. This violates
//  'is a' relationship of inheritance :-(
template<class S, class T>
class smoc_ring_access
: public smoc_channel_access<T> {
public:
  typedef T                     return_type;
  typedef S                     storage_type;
  typedef smoc_ring_access<S,T> this_type;
private:
#ifndef NDEBUG
  size_t        limit;
#endif
  storage_type *storage;
  size_t        storageSize;
  size_t       *offset;
public:
  smoc_ring_access(storage_type *storage, size_t storageSize, size_t *offset):
#ifndef NDEBUG
      limit(0),
#endif
      storage(storage), storageSize(storageSize), offset(offset) {}

#ifndef NDEBUG
  void setLimit(size_t l) { limit = l; }
#endif
  bool tokenIsValid(size_t n) const {
    // ring_access is used in smoc_fifo -> if any (commited) token is invalid,
    // then it is an design failure
    return true;
  }

  virtual void   registerChannelAccessListener(ChannelAccessListener * l){
#ifdef SYSTEMOC_ENABLE_VPC
    for(size_t i = 0; i<storageSize; ++i){
      storage[i].registerChannelAccessListener(l);
    }
#endif // SYSTEMOC_ENABLE_VPC
  }

  return_type operator[](size_t n) {
    // std::cerr << "((smoc_ring_access)" << this << ")->operator[]" << n << ")" << std::endl;
    assert(n < limit);
    return *offset + n < storageSize
      ? storage[*offset + n]
      : storage[*offset + n - storageSize];
  }
  const return_type operator[](size_t n) const {
    // std::cerr << "((smoc_ring_access)" << this << ")->operator[](" << n << ") const" << std::endl;
    assert(n < limit);
    return *offset + n < storageSize
      ? storage[*offset + n]
      : storage[*offset + n - storageSize];
  }
};

template <>
class smoc_ring_access<void, void>
: public smoc_channel_access<void> {
public:
  typedef void                        return_type;
  typedef void                        storage_type;
  typedef smoc_ring_access<void,void> this_type;
private:
public:
  smoc_ring_access()
    {}

#ifndef NDEBUG
  void setLimit(size_t) {}
#endif
  bool tokenIsValid(size_t n) const {
    // ring_access is used in smoc_fifo -> if any (commited) token is invalid,
    // then it is an design failure
    return true;
  }

  virtual void   registerChannelAccessListener(ChannelAccessListener * l){
    return; // no storage, no listener ??
  }
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

#ifdef SYSTEMOC_ENABLE_VPC
  smoc_detail::LatencyQueue   latencyQueue;

  std::list<smoc_ref_event_p> commLatEvents;
#endif

  size_t const fsize;   ///< Ring buffer size == FIFO size + 1
  size_t       rindex;  ///< The FIFO read    ptr
#ifdef SYSTEMOC_ENABLE_VPC
  size_t       vindex;  ///< The FIFO visible ptr
#endif
  size_t       windex;  ///< The FIFO write   ptr
  size_t       tokenId; ///< The tokenId of the next commit token

  smoc_event   eventWrite;
  EventMap     eventMapAvailable;
  smoc_event   eventRead;
  EventMap     eventMapFree;

  size_t usedCount() const {
    size_t used =
      windex - rindex;
    
    if (used > fsize)
      used += fsize;
    return used;
  }

  size_t visibleCount() const {
#ifdef SYSTEMOC_ENABLE_VPC
    size_t used =
      vindex - rindex;
    
    if (used > fsize)
      used += fsize;
    return used;
#else // !SYSTEMOC_ENABLE_VPC
    return usedCount();
#endif
  }

  size_t freeCount() const {
    size_t unused =
      rindex - windex - 1;

    if (unused > fsize)
      unused += fsize;
    return unused;
  }

  size_t inTokenId() const
    { return tokenId - usedCount(); }
  size_t outTokenId() const
    { return tokenId; }

  void incrVisible() {
    size_t used = visibleCount();
    // notify all disabled events for less/equal visibleCount() available tokens
    for (EventMap::const_iterator iter = eventMapAvailable.upper_bound(used);
         iter != eventMapAvailable.begin() && !*(--iter)->second;
         )
      iter->second->notify();
    eventWrite.notify();
  }

  void decrVisible() {
    size_t used = visibleCount();
    // reset all enabled events for more then visibleCount() available tokens
    for (EventMap::const_iterator iter = eventMapAvailable.upper_bound(used);
         iter != eventMapAvailable.end() && *iter->second;
         ++iter)
      iter->second->reset();
  }

  void incrFree() {
    size_t unused = freeCount();
    // notify all disabled events for less/equal freeCount() free space
    for (EventMap::const_iterator iter = eventMapFree.upper_bound(unused);
         iter != eventMapFree.begin() && !*(--iter)->second;
         )
      iter->second->notify();
    eventRead.notify();
  }

  void decrFree() {
    size_t unused = freeCount();
    // reset all enabled events for more then freeCount() free space
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
    
    decrVisible(); incrFree();
  }

#ifdef SYSTEMOC_ENABLE_VPC
  void wpp(size_t n, const smoc_ref_event_p &le)
#else
  void wpp(size_t n)
#endif
  {
    tokenId += n;
    /*if ( windex + n >= fsize )
      windex = windex + n - fsize;
    else
      windex = windex + n;*/
    windex = (windex + n) % fsize;
    
    decrFree();
#ifdef SYSTEMOC_ENABLE_VPC
    assert(commLatEvents.begin()!=commLatEvents.end());
    latencyQueue.addEntry(n, commLatEvents.front());
    commLatEvents.pop_front();
#else
    incrVisible();
#endif
  }

#ifdef SYSTEMOC_ENABLE_VPC
  void latencyExpired(size_t n) {
# ifndef NDEBUG
    size_t oldUsed = visibleCount();
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
    // PARANOIA: visibleCount() must increase
    assert(visibleCount() >= oldUsed);
# endif
    incrVisible();
  }
#endif

  smoc_ref_event_p getLatencyEvent() {
    smoc_ref_event_p commLatEvent(new smoc_ref_event());
    commLatEvents.push_back(commLatEvent);
    return commLatEvent;
  }

  smoc_event &getEventAvailable(size_t n) {
    if (n != MAX_TYPE(size_t)) {
      EventMap::iterator iter = eventMapAvailable.find(n);
      if (iter == eventMapAvailable.end()) {
        iter = eventMapAvailable.insert(EventMap::value_type(n, new smoc_event())).first;
        if (visibleCount() >= n)
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
        if (freeCount() >= n)
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
#ifdef SYSTEMOC_ENABLE_VPC
      latencyQueue(this), 
      commLatEvents(),
#endif
      fsize(i.n+1),
      rindex(0),
#ifdef SYSTEMOC_ENABLE_VPC
      vindex(0), 
#endif
      windex(0),
      tokenId(0)
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
  typedef smoc_storage<data_type>             storage_type;
  typedef smoc_ring_access<
    storage_type,
    typename ring_in_type::return_type>       ring_access_in_type;
  typedef smoc_ring_access<
    storage_type,
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
#ifdef SYSTEMOC_ENABLE_VPC
    this->vindex = this->windex;
#endif
  }

  ring_in_type *getReadChannelAccess() {
    return new ring_access_in_type
      (storage, this->fsize, &this->rindex);
  }
  ring_out_type *getWriteChannelAccess() {
    return new ring_access_out_type
      (storage, this->fsize,  &this->windex);
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

  ~smoc_fifo_storage() { delete[] storage; }
};

template <>
class smoc_fifo_storage<void>
: public smoc_chan_if<smoc_fifo_kind,
          void,
          smoc_channel_access,
          smoc_channel_access> {
public:
  typedef void                          data_type;
  typedef smoc_fifo_storage<data_type>  this_type;
  typedef this_type::access_out_type    ring_out_type;
  typedef this_type::access_in_type     ring_in_type;
  typedef smoc_ring_access<void,void>   ring_access_in_type;
  typedef smoc_ring_access<void,void>   ring_access_out_type;

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
#ifdef SYSTEMOC_ENABLE_VPC
    vindex = windex;
#endif
  }
 
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
class smoc_fifo_type
  : public smoc_fifo_storage<T> {
public:
  typedef T                  data_type;
  typedef smoc_fifo_type<data_type>            this_type;
  
  typedef typename smoc_storage_in<data_type>::storage_type   storage_in_type;
  typedef typename smoc_storage_in<data_type>::return_type    return_in_type;
  
  typedef typename smoc_storage_out<data_type>::storage_type  storage_out_type;
  typedef typename smoc_storage_out<data_type>::return_type   return_out_type;
protected:
  
#ifdef SYSTEMOC_ENABLE_VPC
  void commitRead(size_t consume, const smoc_ref_event_p &le)
#else
  void commitRead(size_t consume)
#endif
  {
#ifdef SYSTEMOC_TRACE
    TraceLog.traceCommExecIn(this, consume);
#endif
    this->rpp(consume);
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
#ifdef SYSTEMOC_ENABLE_VPC
    this->wpp(produce, le);
#else
    this->wpp(produce);
#endif
  }
public:
  // constructors
  smoc_fifo_type( const typename smoc_fifo_storage<T>::chan_init &i )
    : smoc_fifo_storage<T>(i) {}

  // bounce functions
  size_t numAvailable() const
    { return this->visibleCount(); }
  size_t numFree() const
    { return this->freeCount(); }
  size_t inTokenId() const
    { return this->smoc_fifo_storage<T>::inTokenId(); }
  size_t outTokenId() const
    { return this->smoc_fifo_storage<T>::outTokenId(); }
  smoc_event &dataAvailableEvent(size_t n)
    { return this->getEventAvailable(n); }
  smoc_event &spaceAvailableEvent(size_t n)
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
