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

#ifndef _INCLUDED_SMOC_SR_SIGNAL_HPP
#define _INCLUDED_SMOC_SR_SIGNAL_HPP

#include <cosupport/commondefs.h>

#include <smoc_chan_if.hpp>
#include <smoc_storage.hpp>

#include <systemc.h>
#include <vector>
#include <queue>
#include <map>

#include <hscd_tdsim_TraceLog.hpp>

// #include <iostream>
enum SignalState {undefined, defined, absent};

/// Base class of the SR_SIGNAL implementation.
class smoc_sr_signal_kind
: public smoc_nonconflicting_chan {
  friend class smoc_scheduler_top;

public:
  typedef smoc_sr_signal_kind  this_type;

  class chan_init {
    friend class smoc_sr_signal_kind;
  private:
    const char *name;
    size_t      n;
  protected:
    chan_init( const char *name, size_t n )
      : name(name), n(n) {}
  };
protected:
  typedef std::map<size_t, smoc_event *>      EventMap;

  SignalState signalState;
  bool        multipleWrite;

  size_t const fsize;   /// fsize == 1 signal type
  //  size_t       rindex;  ///< The SR_SIGNAL read    ptr
  //  size_t       windex;  ///< The SR_SIGNAL write   ptr

  smoc_event   eventWrite;
  EventMap     eventMapAvailable;
  smoc_event   eventRead;
  EventMap     eventMapFree;

  size_t usedStorage() const {
    size_t ret;
    if(multipleWrite) ret = 1;
    else              ret = (signalState == undefined)?0:1;
    return ret;
  }

  void usedIncr() {
    //std::cerr << __FILE__ << ":" << __LINE__ << ": marker"  << std::endl;
    size_t used = usedStorage();
    // notify all disabled events for less/equal usedStorage() available tokens
    for (EventMap::const_iterator iter = eventMapAvailable.upper_bound(used);
         iter != eventMapAvailable.begin() && !*(--iter)->second;
         ){
      //std::cerr << __FILE__ << ":" << __LINE__ << ": marker"  << std::endl;
      iter->second->notify();
    }
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
    //std::cerr << "unusedStorage()="<< 1-usedStorage()  << std::endl;
    return (signalState == undefined)?1:0;
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
    //std::cerr << "rpp(" << n << ")"  << std::endl;
    assert(n <= 1);
    //    usedDecr(); unusedIncr();
  }

#ifdef ENABLE_SYSTEMC_VPC
  void wpp(size_t n, const smoc_ref_event_p &le)
#else
  void wpp(size_t n)
#endif
  {
    //std::cerr << "wpp(" << n << ")"  << std::endl;
    assert(n <= 1);
    
    if(!multipleWrite){
      this->signalState = defined;
      unusedDecr();
      usedIncr();
    }
  }

  smoc_event &getEventAvailable(size_t n) {
    //std::cerr << "getEventAvailable(" << n << ")"  << std::endl;
    assert(n <= 1);
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
    //std::cerr << "getEventAvailable(" << n << ")"  << std::endl;
    assert(n <= 1);
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
    //std::cerr << __FILE__ << ":" << __LINE__ << ": marker"  << std::endl;
    // Signal has no size!!
    // pgw << "<attribute type=\"size\" value=\"1\"/>" << std::endl;
  }

  virtual
  void channelContents(smoc_modes::PGWriter &pgw) const = 0;

  // constructors
  smoc_sr_signal_kind( const chan_init &i )
    : smoc_nonconflicting_chan(
        i.name != NULL ? i.name : sc_gen_unique_name( "smoc_sr_signal" ) ),
      signalState(undefined),
      multipleWrite(false),
      fsize(1){
  }
private:
  static const char* const kind_string;
  
  virtual const char* kind() const {
    return kind_string;
  }

  void tick(){
    bool needUpdate = (signalState != undefined);
    signalState=undefined;
    if(needUpdate) usedDecr(); unusedIncr(); // update events (storage state changed)
  }
  void multipleWriteSameValue(bool allow){
    multipleWrite = allow;
  }
  // disabled

  smoc_sr_signal_kind( const this_type & );
  this_type& operator = ( const this_type & );
};

template <typename T>
class smoc_sr_signal_storage
//: public smoc_chan_nonconflicting_if<smoc_sr_signal_kind, T> {
: public smoc_chan_if<smoc_sr_signal_kind,T> {
public:
  typedef T                                  data_type;
  typedef smoc_sr_signal_storage<data_type>  this_type;
  typedef typename this_type::iface_out_type iface_out_type;
  typedef typename this_type::iface_in_type  iface_in_type;
  typedef smoc_storage<data_type>	     storage_type;
  
  class chan_init
    : public smoc_sr_signal_kind::chan_init {
    friend class smoc_sr_signal_storage<T>;
  private:
    //FIXME(MS): replace with signal value wrapper
    std::vector<T>  marking;
  protected:
    typedef const T add_param_ty;
  public:
    void add( add_param_ty x ) {
      //FIXME(MS): Signal initialization should be disabled in future!
      std::cerr << "Warning: Signals in synchronous-reactive systems should not be initialized!\n"
		   "A better way for breaking undefined feedback loops is using non-strict blocks like non-strict AND!"
		<< std::endl;
      //FIXME(MS): replace with signal value wrapper
      if(marking.size)marking[0]=x;
      else marking.push_back(x);
    }
  protected:
    chan_init( const char *name, size_t n )
      : smoc_sr_signal_kind::chan_init(name, n) {}
  };
private:
  storage_type *storage;
protected:
  smoc_sr_signal_storage( const chan_init &i )
//  : smoc_chan_nonconflicting_if<smoc_sr_signal_kind, T>(i),
    : smoc_chan_if<smoc_sr_signal_kind,T>(i),
      storage(new storage_type[this->fsize])
  {
    assert(1 >= i.marking.size());
    if(1 == i.marking.size()){
      storage[0].put(i.marking[0]);
      this->signalState = defined;
    }
  }
  
  storage_type *getStorage() const { return storage; }
  
  void channelContents(smoc_modes::PGWriter &pgw) const {
    pgw << "<sr_signal tokenType=\"" << typeid(data_type).name() << "\">" << std::endl;
    {
      //FIXME(MS): Signal initialization should be disabled in future!
      //*************************INITIAL TOKENS, ETC...***************************
      pgw.indentUp();
      for ( size_t n = 0; n < this->usedStorage(); ++n )
        pgw << "<token value=\"" << storage[n].get() << "\"/>" << std::endl;
      pgw.indentDown();
    }
    pgw << "</sr_signal>" << std::endl;
  }

  ~smoc_sr_signal_storage() { delete[] storage; }
};

template <>
class smoc_sr_signal_storage<void>
//: public smoc_chan_nonconflicting_if<smoc_sr_signal_kind, void> {
: public smoc_chan_if<smoc_sr_signal_kind,void> {
public:
  typedef void                                    data_type;
  typedef smoc_sr_signal_storage<data_type>       this_type;
  
  class chan_init
    : public smoc_sr_signal_kind::chan_init {
    friend class smoc_sr_signal_storage<void>;
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
      : smoc_sr_signal_kind::chan_init(name, n),
        marking(0) {}
  };
protected:
  smoc_sr_signal_storage( const chan_init &i )
//  : smoc_chan_nonconflicting_if<smoc_sr_signal_kind, void>(i) {
    : smoc_chan_if<smoc_sr_signal_kind,void>(i) {
    assert( 1 >= i.marking );
    //FIXME (MS) Does an initialised signal<void> equals "defined" or "absent"??
    signalState = defined;
  }
  
  void *getStorage() const { return NULL; }

  void channelContents(smoc_modes::PGWriter &pgw) const {
    pgw << "<sr_signal tokenType=\"" << typeid(data_type).name() << "\">" << std::endl;
    {
      //FIXME(MS): Signal initialization should be disabled in future!
      //*************************INITIAL TOKENS, ETC...***************************
      pgw.indentUp();
      for ( size_t n = 0; n < this->usedStorage(); ++n )
        pgw << "<token value=\"bot\"/>" << std::endl;
      pgw.indentDown();
    }
    pgw << "</sr_signal>" << std::endl;
  }
};

template <typename T>
class smoc_sr_signal_type
  : public smoc_sr_signal_storage<T> {
public:
  typedef T						      data_type;
  typedef smoc_sr_signal_type<data_type>		      this_type;
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
    //std::cerr << "commSetupIn(" << req << ")" << endl;
    assert( req <= this->usedStorage() );
    return ring_in_type(this->getStorage(),
        this->fsize, (this->signalState==undefined)?1:0, req);
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
    std::cerr << "commSetupOut(" << req << ")" << endl;
    assert( req <= this->unusedStorage() );
    return ring_out_type(this->getStorage(),
        this->fsize, (this->signalState==undefined)?0:1, req);

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
  smoc_sr_signal_type( const typename smoc_sr_signal_storage<T>::chan_init &i )
    : smoc_sr_signal_storage<T>(i) {
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
class smoc_sr_signal
  : public smoc_sr_signal_storage<T>::chan_init {
public:
  typedef T                        data_type;
  typedef smoc_sr_signal<T>        this_type;
  typedef smoc_sr_signal_type<T>   chan_type;
  
  this_type &operator <<( typename smoc_sr_signal_storage<T>::chan_init::add_param_ty x ) {
    add(x); return *this;
  }
  
  smoc_sr_signal( )
    : smoc_sr_signal_storage<T>::chan_init( NULL, 1 ) {}
  explicit smoc_sr_signal( const char *name )
    : smoc_sr_signal_storage<T>::chan_init( name, 1 ) {}
};

#endif // _INCLUDED_SMOC_SR_SIGNAL_HPP
