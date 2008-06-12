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

#include <CoSupport/commondefs.h>

#include <systemoc/smoc_config.h>

#include "detail/smoc_root_chan.hpp"
#include "smoc_chan_if.hpp"
#include "smoc_storage.hpp"
#include "detail/smoc_sysc_port.hpp"
#include "detail/EventMapManager.hpp"
#include "smoc_chan_adapter.hpp"

#include <systemc.h>
#include <vector>
#include <queue>
#include <map>

#include "hscd_tdsim_TraceLog.hpp"

enum SignalState {undefined, defined, absent};

/// Base class of the MULTICAST_SR_SIGNAL implementation.
class smoc_multicast_sr_signal_chan_base
: public smoc_multicast_chan {
  typedef smoc_multicast_sr_signal_chan_base  this_type;
public:
  friend class smoc_graph_sr;
  friend class smoc_multicast_outlet_base;
  friend class smoc_multicast_entry_base;

  /// @brief Channel initializer
  class chan_init {
  public:
    friend class smoc_multicast_sr_signal_chan_base;
  protected:
    chan_init(const std::string& name, size_t n);
  private:
    std::string name;
    size_t      n;
  };

protected:
  SignalState getSignalState() const;

  void setSignalState(SignalState s);

  void wpp(size_t n);

  void rpp(size_t n);
  
  void usedDecr();
  
  void usedIncr();

  void unusedDecr();
  
  void unusedIncr();

  bool isDefined() const;

  size_t inTokenId() const
    { return tokenId; }

  size_t outTokenId() const
    { return tokenId; }

  /// @brief See smoc_root_chan
  void channelAttributes(smoc_modes::PGWriter &pgw) const;

  // constructors
  smoc_multicast_sr_signal_chan_base( const chan_init &i );

  /// @brief Bounce to storage
  virtual void reset() = 0;
  
  /// @brief Bounce to storage
  virtual bool isValid() const = 0;

  void tick();
  
private:
  SignalState signalState;
  
  /// @brief The tokenId of the next commit token
  size_t tokenId;

  // disabled
  smoc_multicast_sr_signal_chan_base( const this_type & );
  this_type& operator = ( const this_type & );
};


class smoc_multicast_outlet_base
: virtual public smoc_chan_in_base_if {
  friend class smoc_graph_sr;
public:
  /// @brief Constructor
  smoc_multicast_outlet_base(smoc_multicast_sr_signal_chan_base* chan);

  /// @brief See smoc_chan_in_base_if
#ifdef SYSTEMOC_ENABLE_VPC
  void commitRead(size_t consume, const smoc_ref_event_p &diiEvent)
#else
  void commitRead(size_t consume)
#endif
  {
#ifdef SYSTEMOC_TRACE
    TraceLog.traceCommExecIn(chan, consume);
#endif
    chan->rpp(consume);
  }
  
  /// @brief See smoc_chan_in_base_if
  smoc_event &dataAvailableEvent(size_t n);

  /// @brief See smoc_chan_in_base_if
  size_t numAvailable() const;
  
  /// @brief See smoc_chan_in_base_if
  size_t inTokenId() const
    { return chan->inTokenId(); }

  void allowUndefinedRead(bool allow);
  
  void usedIncr();
  
  void usedDecr();
  
  //void wpp(size_t n);
  
  bool isDefined() const;

private:
  smoc_multicast_sr_signal_chan_base* chan;
  bool undefinedRead;
  Detail::EventMapManager emm;
};

class smoc_multicast_entry_base
: virtual public smoc_chan_out_base_if {
  friend class smoc_graph_sr;
public:
  /// @brief Constructor
  smoc_multicast_entry_base(smoc_multicast_sr_signal_chan_base* chan);

  /// @brief See smoc_chan_out_base_if
#ifdef SYSTEMOC_ENABLE_VPC
  void commitWrite(size_t produce, const smoc_ref_event_p &latEvent)
#else
  void commitWrite(size_t produce)
#endif
  {
#ifdef SYSTEMOC_TRACE
    TraceLog.traceCommExecOut(chan, produce);
#endif
    chan->wpp(produce);
  }

  /// @brief See smoc_chan_out_base_if
  smoc_event &spaceAvailableEvent(size_t n);

  /// @brief See smoc_chan_out_base_if
  size_t numFree() const;

  /// @brief See smoc_chan_out_base_if
  size_t outTokenId() const
    { return chan->outTokenId(); }
  
  void multipleWriteSameValue(bool allow);

  void unusedIncr();
  
  void unusedDecr();

  //void rpp(size_t n);
  
  bool isDefined() const;

private:
  smoc_multicast_sr_signal_chan_base* chan;
  bool multipleWrite;
  Detail::EventMapManager emm;
};

template <typename T>
class smoc_multicast_sr_signal_chan;

template <typename T>
class smoc_multicast_outlet
  : public smoc_chan_in_if<T,smoc_channel_access_if>,
    public smoc_multicast_outlet_base,
    public smoc_channel_access_if<
      typename smoc_chan_in_if<T,smoc_channel_access_if>::access_type::return_type>
{
public:
  typedef T                                       data_type;
  typedef smoc_storage<data_type>                 storage_type;
  typedef smoc_multicast_outlet<data_type>        this_type;
  typedef typename this_type::access_in_type      ring_in_type;
  typedef typename this_type::return_type         return_type;
  typedef smoc_chan_in_if<T,smoc_channel_access_if>  iface_type;
  
  /// @brief Constructor
  smoc_multicast_outlet(smoc_multicast_sr_signal_chan<T>* chan)
    : smoc_multicast_outlet_base(chan),
      chan(chan)
  {}
  
  /// @brief See smoc_chan_in_if
  ring_in_type* getReadChannelAccess()
    { return this; }
  
  /// @brief See smoc_channel_access_if
  return_type operator[](size_t n)
    { return chan->actualValue; }

  /// @brief See smoc_channel_access_if
  const return_type operator[](size_t n) const
    { return chan->actualValue; }
  
  /// @brief See smoc_channel_access_if
  bool tokenIsValid(size_t i) const
    { return chan->isDefined(); }

  /// @brief See smoc_channel_access_if
  void setLimit(size_t l)
    { limit = l; }

  size_t getLimit() const
    { return limit; }

private:
  smoc_multicast_sr_signal_chan<T>* chan;
  size_t limit;
};

template <typename T>
class smoc_multicast_entry
  : public smoc_chan_out_if<T,smoc_channel_access_if>,
    public smoc_multicast_entry_base,
    public smoc_channel_access_if<
      typename smoc_chan_out_if<T,smoc_channel_access_if>::access_type::return_type>
{
public:
  typedef T                                       data_type;
  typedef smoc_multicast_entry<data_type>         this_type;
  typedef smoc_storage<data_type>                 storage_type;
  typedef typename this_type::access_out_type     ring_out_type;
  typedef typename this_type::return_type         return_type;
  typedef smoc_chan_out_if<T,smoc_channel_access_if> iface_type;
  
  /// @brief Constructor
  smoc_multicast_entry(smoc_multicast_sr_signal_chan<T>* chan)
    : smoc_multicast_entry_base(chan),
      chan(chan)
  {}
  
  /// @brief See smoc_chan_out_if
  ring_out_type* getWriteChannelAccess()
    { return this; }

  /// @brief See smoc_channel_access_if
  return_type operator[](size_t n)
    { return chan->actualValue; }

  /// @brief See smoc_channel_access_if
  const return_type operator[](size_t n) const
    { return chan->actualValue; }
  
  /// @brief See smoc_channel_access_if
  bool tokenIsValid(size_t i) const
    { return chan->isDefined(); }

  /// @brief See smoc_channel_access_if
  void setLimit(size_t l)
    { limit = l; }

  size_t getLimit() const
    { return limit; }

private:
  smoc_multicast_sr_signal_chan<T>* chan;
  size_t limit;
};

template <typename T>
class smoc_multicast_sr_signal_chan
  : public smoc_multicast_sr_signal_chan_base {
public:
  typedef T                                         data_type;
  typedef smoc_multicast_sr_signal_chan<data_type>  this_type;
  typedef smoc_storage<data_type>                   storage_type;
  typedef smoc_multicast_outlet<data_type>          Outlet;
  typedef smoc_multicast_entry<data_type>           Entry;
  
  typedef typename Entry::iface_type  entry_iface_type;
  typedef typename Outlet::iface_type outlet_iface_type;

  friend class smoc_multicast_entry<T>;
  friend class smoc_multicast_outlet<T>;

  /// @brief Channel initializer
  class chan_init
    : public smoc_multicast_sr_signal_chan_base::chan_init {
    friend class smoc_multicast_sr_signal_chan<T>;
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
    chan_init( const std::string& name, size_t n )
      : smoc_multicast_sr_signal_chan_base::chan_init(name, n) {}
  };

  /// @brief Constructor
  smoc_multicast_sr_signal_chan( const chan_init &i )
    : smoc_multicast_sr_signal_chan_base(i)
  {
    assert(1 >= i.marking.size());
    if(1 == i.marking.size()){
      actualValue.put(i.marking[0]);
      this->setSignalState(defined);
    }
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
    Entry* e = dynamic_cast<Entry*>(getEntry(&p));
    assert(e); p(*e);
  }

  /// @brief Connect smoc_port_in
  template<class Init>
  void connect(smoc_port_in<data_type>& p, const Init&) {
    Outlet* o = dynamic_cast<Outlet*>(getOutlet(&p));
    assert(o); p(*o);
  }

protected:
  storage_type actualValue;

  /// @brief See smoc_port_registry
  smoc_chan_out_base_if* createEntry()
    { return new Entry(this); }

  /// @brief See smoc_port_registry
  smoc_chan_in_base_if* createOutlet()
    { return new Outlet(this); }

  void setChannelID( std::string sourceActor,
                             CoSupport::SystemC::ChannelId id,
                             std::string name ){
    this->actualValue.setChannelID(sourceActor, id, name);
  }

  // bounce functions to storage
  void reset()
    { actualValue.reset(); }
  bool isValid() const
    { return actualValue.isValid(); }

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
  : public smoc_multicast_sr_signal_chan<T>::chan_init {
public:
  typedef T                                  data_type;
  typedef smoc_multicast_sr_signal<T>        this_type;
  typedef smoc_multicast_sr_signal_chan<T>   chan_type;
  
  this_type &operator <<
    (typename smoc_multicast_sr_signal_chan<T>::chan_init::add_param_ty x){
    add(x); return *this;
  }
  
  smoc_multicast_sr_signal( )
    : smoc_multicast_sr_signal_chan<T>::chan_init( "", 1 )
  {}

  explicit smoc_multicast_sr_signal( const std::string& name )
    : smoc_multicast_sr_signal_chan<T>::chan_init( name, 1 )
  {}
};

#endif // _INCLUDED_SMOC_MULTICAST_SR_SIGNAL_HPP
