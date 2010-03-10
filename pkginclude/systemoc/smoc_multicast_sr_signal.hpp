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

#ifndef _INCLUDED_SMOC_MULTICAST_SR_SIGNAL_HPP
#define _INCLUDED_SMOC_MULTICAST_SR_SIGNAL_HPP

#include <vector>
#include <queue>
#include <map>

#include <systemc.h>

#include <CoSupport/commondefs.h>

#include <systemoc/smoc_config.h>

#include "detail/smoc_root_chan.hpp"
#include "detail/smoc_chan_if.hpp"
#include "detail/smoc_storage.hpp"
#include "detail/smoc_sysc_port.hpp"
#include "detail/ConnectProvider.hpp"
#include "detail/EventMapManager.hpp"
#include "smoc_chan_adapter.hpp"
#include "detail/hscd_tdsim_TraceLog.hpp"

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

//method made public to access it from smoc_register
  void setSignalState(SignalState s);

protected:
  SignalState getSignalState() const;

  void wpp(size_t n);

  void rpp(size_t n);
  
  /// @brief Called when more data is available
  void moreData();
  
  /// @brief Called when less data is available
  void lessData();
  
  /// @brief Called when more space is available
  void moreSpace();
  
  /// @brief Called when less space is available
  void lessSpace();

  bool isDefined() const;

  size_t inTokenId() const
    { return tokenId; }

  size_t outTokenId() const
    { return tokenId; }

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


class smoc_multicast_outlet_base {
  friend class smoc_graph_sr;
public:
  virtual ~smoc_multicast_outlet_base() {}
protected:
  virtual void allowUndefinedRead(bool allow) = 0;
  virtual bool isDefined() const = 0;
};

class smoc_multicast_entry_base {
  friend class smoc_graph_sr;  
public:
  virtual ~smoc_multicast_entry_base() {}
protected:
  virtual void multipleWriteSameValue(bool allow) = 0;
  virtual bool isDefined() const = 0;
};

template <typename T>
class smoc_multicast_sr_signal_chan;

template <typename T>
class smoc_multicast_outlet
  : public smoc_multicast_outlet_base,
    public smoc_port_in_if<T,::smoc_1d_port_access_if>,
    public smoc_1d_port_access_if<typename smoc_storage_in<T>::return_type>
{
public:
  typedef T                                       data_type;
  typedef smoc_storage<data_type>                 storage_type;
  typedef smoc_multicast_outlet<data_type>        this_type;
  typedef typename this_type::access_in_type      ring_in_type;
  typedef typename this_type::return_type         return_type;
  typedef smoc_port_in_if<T,::smoc_1d_port_access_if>  iface_type;
  
  /// @brief Constructor
  smoc_multicast_outlet(smoc_multicast_sr_signal_chan<T>* chan)
    : chan(chan),
      undefinedRead(false)
  {}
  
  /// @brief See smoc_port_in_base_if
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
  
  /// @brief See smoc_port_in_base_if
  smoc_event &dataAvailableEvent(size_t n) {
    assert(n <= 1);
    return emm.getEvent(numAvailable(), n);
  }

  /// @brief See smoc_port_in_base_if
  size_t numAvailable() const
    { return (undefinedRead || chan->getSignalState() != undefined) ? 1 : 0; }
  
  /// @brief See smoc_port_in_base_if
  size_t inTokenId() const
    { return chan->inTokenId(); }
  
  /// @brief See smoc_port_in_base_if
  void moreData(size_t)
    { emm.increasedCount(numAvailable()); }

  /// @brief See smoc_port_in_base_if
  void lessData(size_t)
    { emm.decreasedCount(numAvailable()); }

  /// @brief See smoc_multicast_outlet_base
  void allowUndefinedRead(bool allow) {
    undefinedRead = allow;
    chan->lessSpace();
   //it was moreData(), which isn't defined here. 
    moreData(0);
  }
  
  /// @brief See smoc_multicast_outlet_base
  bool isDefined() const
    { return chan->isDefined(); }
  
  /// @brief See smoc_port_in_if
  ring_in_type* getReadPortAccess()
    { return this; }
  
  /// @brief See smoc_1d_port_access_if
  return_type operator[](size_t n)
    { return chan->actualValue; }

  /// @brief See smoc_1d_port_access_if
  const return_type operator[](size_t n) const
    { return chan->actualValue; }
  
  /// @brief See smoc_1d_port_access_if
  bool tokenIsValid(size_t i) const
    { return chan->isDefined(); }

  /// @brief See smoc_1d_port_access_if
  void setLimit(size_t l)
    { limit = l; }

  size_t getLimit() const
    { return limit; }

private:
  smoc_multicast_sr_signal_chan<T>* chan;
  bool undefinedRead;
  Detail::EventMapManager emm;
  size_t limit;
};

template <typename T>
class smoc_multicast_entry
: public smoc_multicast_entry_base,
  public smoc_port_out_if<T,::smoc_1d_port_access_if>,
  public smoc_1d_port_access_if<typename smoc_storage_out<T>::return_type>
{
public:
  typedef T                                       data_type;
  typedef smoc_multicast_entry<data_type>         this_type;
  typedef smoc_storage<data_type>                 storage_type;
  typedef typename this_type::access_out_type     ring_out_type;
  typedef typename this_type::return_type         return_type;
  typedef smoc_port_out_if<T,::smoc_1d_port_access_if> iface_type;
  
  /// @brief Constructor
  smoc_multicast_entry(smoc_multicast_sr_signal_chan<T>* chan)
    : chan(chan),
      multipleWrite(false)
  {}
  
  /// @brief See smoc_port_out_base_if
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

  /// @brief See smoc_port_out_base_if
  smoc_event &spaceAvailableEvent(size_t n) {
    assert(n <= 1);
    return emm.getEvent(numFree(), n);
  }

  /// @brief See smoc_port_out_base_if
  size_t numFree() const
    { return (multipleWrite || chan->getSignalState() == undefined) ? 1 : 0; }
  
  /// @brief See smoc_port_out_base_if
  size_t outTokenId() const
    { return chan->outTokenId(); }
  
  /// @brief See smoc_port_out_base_if
  void moreSpace(size_t)
    { emm.increasedCount(numFree()); }
  
  /// @brief See smoc_port_out_base_if
  void lessSpace(size_t)
    { emm.decreasedCount(numFree()); }
  
  /// @brief See smoc_multicast_entry_base
  void multipleWriteSameValue(bool allow)
    { multipleWrite = allow; }

  /// @brief See smoc_multicast_entry_base
  bool isDefined() const
    { return chan->isDefined(); }
  
  /// @brief See smoc_port_out_if
  ring_out_type* getWritePortAccess()
    { return this; }

  /// @brief See smoc_1d_port_access_if
  return_type operator[](size_t n)
    { return chan->actualValue; }

  /// @brief See smoc_1d_port_access_if
  const return_type operator[](size_t n) const
    { return chan->actualValue; }
  
  /// @brief See smoc_1d_port_access_if
  bool tokenIsValid(size_t i) const
    { return chan->isDefined(); }

  /// @brief See smoc_1d_port_access_if
  void setLimit(size_t l)
    { limit = l; }

  size_t getLimit() const
    { return limit; }

private:
  smoc_multicast_sr_signal_chan<T>* chan;
  bool multipleWrite;
  Detail::EventMapManager emm;
  size_t limit;
};

template <typename T>
class smoc_multicast_sr_signal_chan
  : public smoc_multicast_sr_signal_chan_base {
public:
  typedef T                                         data_type;
  typedef smoc_multicast_sr_signal_chan<data_type>  this_type;
  typedef smoc_storage<data_type>                   storage_type;
  typedef smoc_multicast_outlet<data_type>          outlet_type;
  typedef smoc_multicast_entry<data_type>           entry_type;
  
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
    typedef const T &add_param_ty;
  public:
    void add(add_param_ty x) {
      //FIXME(MS): Signal initialization should be disabled in future!
      /*std::cerr << "Warning: Signals in synchronous-reactive systems should"
             " not be initialized!\n"
                   "A better way for breaking undefined feedback loops is"
             " using non-strict blocks like non-strict AND!"
                << std::endl;*/
      //FIXME(MS): replace with signal value wrapper
      if(marking.size())marking[0]=x;
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
protected:
  storage_type actualValue;

  /// @brief See smoc_port_registry
  smoc_port_out_base_if* createEntry()
    { return new entry_type(this); }

  /// @brief See smoc_port_registry
  smoc_port_in_base_if* createOutlet()
    { return new outlet_type(this); }

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
};

template <typename T>
class smoc_multicast_sr_signal
: public smoc_multicast_sr_signal_chan<T>::chan_init,
  public SysteMoC::Detail::ConnectProvider<
    smoc_multicast_sr_signal<T>,
    smoc_multicast_sr_signal_chan<T> > {
  typedef smoc_multicast_sr_signal<T> this_type;

  friend class SysteMoC::Detail::ConnectProvider<this_type, typename this_type::chan_type>;
public:
  typedef T                             data_type;
  typedef typename this_type::chan_type chan_type;
private:
  chan_type *chan;
public:
  smoc_multicast_sr_signal( )
    : smoc_multicast_sr_signal_chan<T>::chan_init("", 1), chan(NULL)
  {}

  explicit smoc_multicast_sr_signal( const std::string& name )
    : smoc_multicast_sr_signal_chan<T>::chan_init(name, 1), chan(NULL)
  {}

  /// @brief Constructor
  smoc_multicast_sr_signal(const this_type &x)
    : smoc_multicast_sr_signal<T>::chan_init(x), chan(NULL)
  {}

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
  chan_type *getChan() {
    if (chan == NULL)
      chan = new chan_type(*this);
    return chan;
  }

  // disable
  this_type &operator =(const this_type &);
};

#endif // _INCLUDED_SMOC_MULTICAST_SR_SIGNAL_HPP
