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

#include "smoc_chan_if.hpp"
#include "smoc_storage.hpp"

#include <systemc.h>
#include <vector>
#include <queue>
#include <map>

#include "hscd_tdsim_TraceLog.hpp"

enum SignalState {undefined, defined, absent};

//forw. decl.
template <typename T>
class smoc_multicast_sr_signal_type;

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
    chan_init( const char *name, size_t n );
  };
public:
  SignalState getSignalState() const;

  void setSignalState(SignalState s);

#ifdef ENABLE_SYSTEMC_VPC
  virtual void wpp(size_t n, const smoc_ref_event_p &le) = 0;
#else
  virtual void wpp(size_t n) = 0;
#endif

  virtual void rpp(size_t n) = 0;

  virtual void unusedDecr() = 0;

  bool isDefined();
protected:
  SignalState signalState;

  void channelAttributes(smoc_modes::PGWriter &pgw) const;

  virtual
  void channelContents(smoc_modes::PGWriter &pgw) const = 0;

  // constructors
  smoc_multicast_sr_signal_kind( const chan_init &i );
private:
  static const char* const kind_string;
  
  virtual const char* kind() const;
  
  virtual void reset()=0;

  virtual void tick() = 0;

  // disabled
  smoc_multicast_sr_signal_kind( const this_type & );
  this_type& operator = ( const this_type & );
};


class smoc_outlet_kind
  : public sc_interface {
  friend class smoc_scheduler_top;
public:
  smoc_outlet_kind(smoc_multicast_sr_signal_kind* base);

  void usedDecr();

#ifdef ENABLE_SYSTEMC_VPC
  void wpp(size_t n, const smoc_ref_event_p &le);
#else
  void wpp(size_t n);
#endif

protected:
  bool undefinedRead;
  smoc_multicast_sr_signal_kind* _base;

  size_t usedStorage() const;

  void usedIncr();


  smoc_event &getEventAvailable(size_t n);

  bool isDefined();

private:
  typedef std::map<size_t, smoc_event *>      EventMap;

  EventMap     eventMapAvailable;
  smoc_event   eventWrite;

  void allowUndefinedRead(bool allow);
};

class smoc_entry_kind
  : public sc_interface {
  friend class smoc_scheduler_top;
public:
  smoc_entry_kind(smoc_multicast_sr_signal_kind* base);

  void unusedIncr();

  void unusedDecr();

  void rpp(size_t n);
protected:
  bool multipleWrite;
  smoc_multicast_sr_signal_kind* _base;

  size_t unusedStorage() const;

  smoc_event &getEventFree(size_t n);

  bool isDefined();
private:
  typedef std::map<size_t, smoc_event *>      EventMap;

  smoc_event   eventRead;
  EventMap     eventMapFree;

  void multipleWriteSameValue(bool allow);
};



template <typename T>
class smoc_multicast_outlet
  : public smoc_chan_in_if<T, smoc_channel_access>,
    public smoc_outlet_kind,
    public smoc_channel_access<
  typename smoc_chan_in_if<T, smoc_channel_access>::access_type::storage_type,
  typename smoc_chan_in_if<T, smoc_channel_access>::access_type::return_type>
{
  typedef T                                  data_type;
  typedef smoc_storage<data_type>       storage_type;
  typedef smoc_multicast_outlet<data_type>   this_type;
  typedef typename this_type::access_in_type ring_in_type;
  typedef typename this_type::return_type    return_type;
public:
  smoc_multicast_outlet(smoc_multicast_sr_signal_kind* base,
      storage_type &actualValue)
    : smoc_outlet_kind(base),
      actualValue(actualValue) {
    assert(this->_base);
  }

  return_type operator[](size_t n){
    return actualValue;
  }

  const return_type operator[](size_t n) const{
    assert(0); //should never be called on an input port
    return actualValue;
  }

  // bounce functions
  size_t numAvailable() const
    { return this->usedStorage(); }
  smoc_event &dataAvailableEvent(size_t n)
    { return this->getEventAvailable(n); }

  ring_in_type * getWriteChannelAccess() {
    return this;
  }

#ifdef ENABLE_SYSTEMC_VPC
  void commitRead(size_t consume, const smoc_ref_event_p &le)
#else
  void commitRead(size_t consume)
#endif
  {
#ifdef SYSTEMOC_TRACE
    TraceLog.traceCommExecIn(consume, this->name());
#endif
    this->_base->rpp(consume);
  }
  
  // smoc_channel_access interface
  void   setLimit(size_t l){
    limit=l;
  }

  size_t getLimit() const{
    return limit;
  }

  virtual bool tokenIsValid(size_t i){
    return this->isDefined();
  }

private:
  size_t         limit;
  storage_type &actualValue;

  // disabled
  const sc_event& default_event() const { return smoc_default_event_abort(); }
};

template <typename T>
class smoc_multicast_entry
  : public smoc_chan_out_if<T, smoc_channel_access>,
    public smoc_entry_kind,
    public smoc_channel_access<
  typename smoc_chan_out_if<T, smoc_channel_access>::access_type::storage_type,
  typename smoc_chan_out_if<T, smoc_channel_access>::access_type::return_type>
{
  typedef T                                  data_type;
  typedef smoc_multicast_entry<data_type>   this_type;
  typedef smoc_storage<data_type>       storage_type;
  typedef typename this_type::access_out_type  ring_out_type;
  typedef typename this_type::return_type      return_type;
public:
  smoc_multicast_entry(smoc_multicast_sr_signal_kind* base,
      storage_type &actualValue)
    : smoc_entry_kind(base),
      actualValue(actualValue) {}

  return_type operator[](size_t n){
    return actualValue;
  }


  const return_type operator[](size_t n) const{
    return actualValue;
  }

  size_t numFree() const
    { return this->unusedStorage(); }
  smoc_event &spaceAvailableEvent(size_t n)
    { return this->getEventFree(n); }
  virtual ring_out_type * getReadChannelAccess() {
    return this;
  }
#ifdef ENABLE_SYSTEMC_VPC
  void commitWrite(size_t produce, const smoc_ref_event_p &le) {
    if( this->actualValue.isValid() ) this->_base->wpp(produce, le);
#else
  void commitWrite(size_t produce) {
    if( this->actualValue.isValid() ) this->_base->wpp(produce);
#endif
#ifdef SYSTEMOC_TRACE
    TraceLog.traceCommExecOut(produce, this->name());
#endif
  }

  // smoc_channel_access interface
  void   setLimit(size_t l){
    limit=l;
  }

  size_t getLimit() const{
    return limit;
  }

  virtual bool tokenIsValid(size_t i){
    return this->isDefined();
  }

private:
  size_t         limit;
  storage_type &actualValue;

  // disabled
  const sc_event& default_event() const { return smoc_default_event_abort(); }
};

template <typename T>
class smoc_multicast_sr_signal_type
  : public smoc_multicast_sr_signal_kind {
public:
  class chan_init
    : public smoc_multicast_sr_signal_kind::chan_init {
    friend class smoc_multicast_sr_signal_type<T>;
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




public:
  typedef T                                  data_type;
  typedef smoc_multicast_sr_signal_type<data_type>  this_type;
  typedef smoc_storage<data_type>       storage_type;
  typedef smoc_port_in_base<data_type,
          smoc_channel_access,
          void>            Port;
  typedef smoc_multicast_outlet<data_type>   Outlet;
  typedef smoc_multicast_entry<data_type>    Entry;
  typedef std::map< const Port* , Outlet* >  OutletMap;

  smoc_multicast_sr_signal_type( const chan_init &i )
    : smoc_multicast_sr_signal_kind(i),
      entry(this, actualValue)
  {
    assert(1 >= i.marking.size());
    if(1 == i.marking.size()){
      actualValue.put(i.marking[0]);
      this->signalState = defined;
    }
  }

  this_type & connect(smoc_port_in<data_type> &inPort){
    inPort(this->getOutlet(inPort));
    return *this;

  }

  Entry& getEntry(){
    return entry;
  }

  Outlet& getOutlet(const Port &port){
    if(outlets.find(&port) == outlets.end()){
      //cout << "Create new Outlet!!" << endl;
      Outlet* out = new Outlet(this, actualValue);
      assert(out);
      outlets[&port] = out;
    }
    assert(outlets.find(&port) != outlets.end());
    return *(outlets[&port]);
  }

#ifdef ENABLE_SYSTEMC_VPC
  void wpp(size_t n, const smoc_ref_event_p &le)
#else
  void wpp(size_t n)
#endif
  { 
    for(typename OutletMap::iterator iter = outlets.begin();
  iter != outlets.end();
  iter++){
#ifdef ENABLE_SYSTEMC_VPC
      iter->second->wpp(n,le);
#else
      iter->second->wpp(n);
#endif
    }
  }

  void rpp(size_t n){
    entry.rpp(n);
  }

  void unusedDecr(){
    entry.unusedDecr();
  }

protected:
  storage_type   actualValue;

private:
  Entry  entry;
  OutletMap outlets;

  void reset(){
    actualValue = storage_type();
  }

  void tick(){
    bool needUpdate = (signalState != undefined);
    signalState=undefined;
    if(needUpdate){
      // update events (storage state changed)
      for(typename OutletMap::iterator iter = outlets.begin();
    iter != outlets.end();
    iter++){
  iter->second->usedDecr();
      }
      entry.unusedIncr();
    }
    this->reset();
  }
protected:
  void channelContents(smoc_modes::PGWriter &pgw) const {
    pgw << "<sr_signal tokenType=\"" << typeid(data_type).name() << "\">"
  << std::endl;
    {
      //FIXME(MS): Signal initialization should be disabled in future!
      //*************************INITIAL TOKENS, ETC...************************
      pgw.indentUp();
      //      for ( size_t n = 0; n < this->usedStorage(); ++n )
      if(this->getSignalState())
        pgw << "<token value=\"" << actualValue.get() << "\"/>" << std::endl;
      pgw.indentDown();
    }
    pgw << "</sr_signal>" << std::endl;
  }
};

template <typename T>
class smoc_multicast_sr_signal
  : public smoc_multicast_sr_signal_type<T>::chan_init {
public:
  typedef T                        data_type;
  typedef smoc_multicast_sr_signal<T>        this_type;
  typedef smoc_multicast_sr_signal_type<T>   chan_type;
  
  this_type &operator <<
    (typename smoc_multicast_sr_signal_type<T>::chan_init::add_param_ty x){
    add(x); return *this;
  }

  chan_type &connect(smoc_port_out<data_type> &outPort){
    chan_type *chan = new chan_type(*this);
    outPort(chan->getEntry());
    return *chan;
  }
  
  smoc_multicast_sr_signal( )
    : smoc_multicast_sr_signal_type<T>::chan_init( NULL, 1 ) {}
  explicit smoc_multicast_sr_signal( const char *name )
    : smoc_multicast_sr_signal_type<T>::chan_init( name, 1 ) {}
};

#endif // _INCLUDED_SMOC_MULTICAST_SR_SIGNAL_HPP
