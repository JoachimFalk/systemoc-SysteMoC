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

#ifndef _INCLUDED_SMOC_MULTICAST_SR_SIGNAL_HPP
#define _INCLUDED_SMOC_MULTICAST_SR_SIGNAL_HPP

#include <cosupport/commondefs.h>

#include <smoc_chan_if.hpp>
#include <smoc_storage.hpp>
#include <smoc_sr_signal.hpp>

#include <systemc.h>
#include <vector>
#include <queue>
#include <map>

#include <hscd_tdsim_TraceLog.hpp>

template <typename T>
class smoc_multicast_entry {
  typedef T                                  data_type;
  typedef smoc_storage<data_type>	     storage_type;
private:
  storage_type &actualValue;
public:
  smoc_multicast_entry(storage_type &actualValue)
    : actualValue(actualValue){}

  smoc_storage<data_type>& operator[](size_t n){
    return actualValue;
  }
};

template <typename T>
class smoc_multicast_outlet {
  typedef T                                  data_type;
  typedef smoc_storage<data_type>	     storage_type;
private:
  storage_type &actualValue;
public:
  smoc_multicast_outlet(storage_type &actualValue)
    : actualValue(actualValue){}
  const smoc_storage<data_type> operator[](size_t n) const{
    return actualValue;
  }
};



/// Base class of the MULTICAST_SR_SIGNAL implementation.
class smoc_multicast_sr_signal_kind
: public smoc_multicast_chan {
  friend class smoc_scheduler_top;

public:
  typedef smoc_multicast_sr_signal_kind  this_type;

  class chan_init {
    friend class smoc_multicast_sr_signal_kind;
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
  bool        undefinedRead;

  size_t const fsize;   /// fsize == 1 signal type
  //  size_t       rindex;  ///< The SR_SIGNAL read    ptr
  //  size_t       windex;  ///< The SR_SIGNAL write   ptr

  smoc_event   eventWrite;
  EventMap     eventMapAvailable;
  smoc_event   eventRead;
  EventMap     eventMapFree;

  size_t usedStorage() const {
    size_t ret;
    if(/*multipleWrite ||*/ undefinedRead) ret = 1;
    else              ret = (signalState == undefined)?0:1;
    return ret;
  }

  void usedIncr() {
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
    if(multipleWrite) return 1;
    else              return (signalState == undefined)?1:0;
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
    //    if(signalState == undefined) std::cerr << "undefinedRead: ";
    //    std::cerr << "rpp(" << n << ")"  << std::endl;
    assert(n <= 1);
    //    usedDecr(); unusedIncr();
  }

#ifdef ENABLE_SYSTEMC_VPC
  void wpp(size_t n, const smoc_ref_event_p &le)
#else
  void wpp(size_t n)
#endif
  {
    //    std::cerr << "wpp(" << n << ")"  << std::endl;
    assert(n <= 1);
    
    this->signalState = defined;
    //unusedDecr();
    usedIncr();
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
  smoc_multicast_sr_signal_kind( const chan_init &i )
    : smoc_multicast_chan(
        i.name != NULL ? i.name
	: sc_gen_unique_name( "smoc_multicast_sr_signal" ) ),
      signalState(undefined),
      multipleWrite(false),
      undefinedRead(false),
      fsize(1){
  }
private:
  static const char* const kind_string;
  
  virtual const char* kind() const {
    return kind_string;
  }
  
  bool isDefined(){
    return (signalState == defined);
  }

  virtual void reset()=0;

  void tick(){
    bool needUpdate = (signalState != undefined);
    signalState=undefined;
    if(needUpdate){
      usedDecr(); unusedIncr(); // update events (storage state changed)
    }
    this->reset();
  }

  void multipleWriteSameValue(bool allow){
    multipleWrite = allow;
  }

  void allowUndefinedRead(bool allow){
    undefinedRead = allow;
    //  signalState=defined;
    unusedDecr();
    usedIncr();

  }

  // disabled
  smoc_multicast_sr_signal_kind( const this_type & );
  this_type& operator = ( const this_type & );
};

template <typename T>
class smoc_multicast_sr_signal_storage
  : public smoc_chan_if<smoc_multicast_sr_signal_kind,T>,
    public smoc_channel_access<T> {
public:
  typedef T                                  data_type;
  typedef smoc_multicast_sr_signal_storage<data_type>  this_type;
  typedef typename this_type::iface_out_type iface_out_type;
  typedef typename this_type::ring_out_type  ring_out_type;
  typedef typename this_type::iface_in_type  iface_in_type;
  typedef typename this_type::ring_in_type   ring_in_type;
  typedef smoc_storage<data_type>	     storage_type;
public: // smoc_channel_access interface

  void   setLimit(size_t l){
    limit=l;
  }

  size_t getLimit() const{
    return limit;
  }

  smoc_storage<data_type>& operator[](size_t n){
    return actualValue;
  }

  const smoc_storage<data_type> operator[](size_t n) const{
    return actualValue;
  }
protected:
  //FIXME(MS): is there a need for making edge detection in SR
  //           or is it part of the application 
  //  storage_type   lastValue;

  storage_type   actualValue;
  size_t         limit;
  //std::list<smoc_multicast_outlet<data_type> > outlets;
  //smoc_multicast_entry<data_type>              entry;

private:

  void reset(){
    //lastValue = actualValue;
    actualValue = storage_type();
  }

public:
  class chan_init
    : public smoc_multicast_sr_signal_kind::chan_init {
    friend class smoc_multicast_sr_signal_storage<T>;
  private:
    //FIXME(MS): replace with signal value wrapper
    std::vector<T>  marking;
  protected:
    typedef const T add_param_ty;
  public:
    void add( add_param_ty x ) {
      //FIXME(MS): Signal initialization should be disabled in future!
      std::cerr << "Warning: Signals in synchronous-reactive systems should"
	           " not be initialized!\n"
                   "A better way for breaking undefined feedback loops is"
	           " using non-strict blocks like non-strict AND!"
                << std::endl;
      //FIXME(MS): replace with signal value wrapper
      if(marking.size)marking[0]=x;
      else marking.push_back(x);
    }
  protected:
    chan_init( const char *name, size_t n )
      : smoc_multicast_sr_signal_kind::chan_init(name, n) {}
  };
protected:
  smoc_multicast_sr_signal_storage( const chan_init &i )
    : smoc_chan_if<smoc_multicast_sr_signal_kind,T>(i)
  {
    assert(1 >= i.marking.size());
    if(1 == i.marking.size()){
      (*this)[0].put(i.marking[0]);
      this->signalState = defined;
    }
  }

  ring_in_type * ringSetupIn() {
    return this;
  }
  ring_out_type * ringSetupOut() {
    return this;
  }

  void channelContents(smoc_modes::PGWriter &pgw) const {
    pgw << "<sr_signal tokenType=\"" << typeid(data_type).name() << "\">"
	<< std::endl;
    {
      //FIXME(MS): Signal initialization should be disabled in future!
      //*************************INITIAL TOKENS, ETC...************************
      pgw.indentUp();
      for ( size_t n = 0; n < this->usedStorage(); ++n )
        pgw << "<token value=\"" << (*this)[n] << "\"/>" << std::endl;
      pgw.indentDown();
    }
    pgw << "</sr_signal>" << std::endl;
  }

  ~smoc_multicast_sr_signal_storage() { }

};

/*
template <>
class smoc_multicast_sr_signal_storage<void>
//: public smoc_chan_multicast_if<smoc_multicast_sr_signal_kind, void> {
  : public smoc_chan_if<smoc_multicast_sr_signal_kind,void>,
    public smoc_channel_access<void> {
public:
  typedef void                                    data_type;
  typedef smoc_multicast_sr_signal_storage<data_type>       this_type;

public: // smoc_channel_access interface

  void   setLimit(size_t l){
    limit=l;
  }

  size_t getLimit() const{
    return limit;
  }
  
private:
  size_t         limit;

public:
  class chan_init
    : public smoc_multicast_sr_signal_kind::chan_init {
    friend class smoc_multicast_sr_signal_storage<void>;
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
      : smoc_multicast_sr_signal_kind::chan_init(name, n),
        marking(0) {}
  };
protected:
  smoc_multicast_sr_signal_storage( const chan_init &i )
    : smoc_chan_if<smoc_multicast_sr_signal_kind,void>(i) {
    assert( 1 >= i.marking );
    //FIXME (MS) Does an initialised signal<void> equals "defined" or "absent"?
    signalState = defined;
  }
 
  ring_in_type  * ringSetupIn()  {
    smoc_ring_access<void, void> *r = new smoc_ring_access<void, void>();
    return r;
  }
  ring_out_type * ringSetupOut() {
    smoc_ring_access<void, void> *r = new smoc_ring_access<void, void>();
    return r;
  }

  void channelContents(smoc_modes::PGWriter &pgw) const {
    pgw << "<sr_signal tokenType=\"" << typeid(data_type).name() << "\">"
        << std::endl;
    {
      //FIXME(MS): Signal initialization should be disabled in future!
      // *************************INITIAL TOKENS, ETC...******************
      pgw.indentUp();
      for ( size_t n = 0; n < this->usedStorage(); ++n )
        pgw << "<token value=\"bot\"/>" << std::endl;
      pgw.indentDown();
    }
    pgw << "</sr_signal>" << std::endl;
  }
};
*/

template <typename T>
class smoc_multicast_sr_signal_type
  : public smoc_multicast_sr_signal_storage<T> {
public:
  typedef T						      data_type;
  typedef smoc_multicast_sr_signal_type<data_type>	      this_type;
  typedef typename this_type::iface_in_type		      iface_in_type;
  typedef typename this_type::iface_out_type		      iface_out_type;
  
  typedef typename smoc_storage_in<data_type>::storage_type   storage_in_type;
  typedef typename smoc_storage_in<data_type>::return_type    return_in_type;
  
  typedef typename smoc_storage_out<data_type>::storage_type  storage_out_type;
  typedef typename smoc_storage_out<data_type>::return_type   return_out_type;
protected:
//  iface_in_type  *in;
//  iface_out_type *out;
  
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
    if( this->actualValue.isValid() ) this->wpp(produce, le);
#else
    if( this->actualValue.isValid() ) this->wpp(produce);
#endif
//  this->write_event.notify();
//  if (this->unusedStorage() < 1)
//    this->read_event.reset();
  }
public:
  // constructors
  smoc_multicast_sr_signal_type( const typename smoc_multicast_sr_signal_storage<T>::chan_init &i )
    : smoc_multicast_sr_signal_storage<T>(i) {
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
class smoc_multicast_sr_signal
  : public smoc_multicast_sr_signal_storage<T>::chan_init {
public:
  typedef T                        data_type;
  typedef smoc_multicast_sr_signal<T>        this_type;
  typedef smoc_multicast_sr_signal_type<T>   chan_type;
  
  this_type &operator <<
    (typename smoc_multicast_sr_signal_storage<T>::chan_init::add_param_ty x){
    add(x); return *this;
  }
  
  smoc_multicast_sr_signal( )
    : smoc_multicast_sr_signal_storage<T>::chan_init( NULL, 1 ) {}
  explicit smoc_multicast_sr_signal( const char *name )
    : smoc_multicast_sr_signal_storage<T>::chan_init( name, 1 ) {}
};

#endif // _INCLUDED_SMOC_MULTICAST_SR_SIGNAL_HPP
