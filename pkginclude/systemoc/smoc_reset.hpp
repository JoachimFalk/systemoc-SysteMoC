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

#ifndef _INCLUDED_SMOC_RESET_HPP
#define _INCLUDED_SMOC_RESET_HPP

#include <vector>
#include <queue>
#include <map>

#include <systemc>

#include <CoSupport/commondefs.h>

#include <systemoc/smoc_config.h>

#include "smoc_chan_adapter.hpp"
#include "smoc_fifo.hpp"
#include "smoc_multireader_fifo.hpp"
#include "smoc_multiplex_fifo.hpp"

#include "detail/smoc_root_chan.hpp"
#include "detail/smoc_chan_if.hpp"
#include "detail/smoc_storage.hpp"
#include "detail/smoc_sysc_port.hpp"
#include <smoc/smoc_event.hpp>
#include <smoc/detail/ConnectProvider.hpp>
#include <smoc/detail/EventMapManager.hpp>

class smoc_reset_outlet;
class smoc_reset_entry;

class smoc_reset_chan
: public smoc_root_chan {
public:
  friend class smoc_reset_entry;
  friend class smoc_reset_outlet;
  friend class smoc_reset_net;
  
  typedef void               data_type;
  typedef smoc_reset_chan    this_type;
  typedef smoc_reset_entry   entry_type;
  typedef smoc_reset_outlet  outlet_type;

  /// @brief Channel initializer
  class chan_init {
  public:
    friend class smoc_reset_chan;
  protected:
    chan_init(const std::string& name);
  private:
    std::string name;
  };

protected:
  void doReset();

  size_t inTokenId() const
    { return tokenId - 1; }

  size_t outTokenId() const
    { return tokenId; }

  // constructors
  smoc_reset_chan( const chan_init &i );
  
  smoc::smoc_event& spaceAvailableEvent()
    { return sae; }

  smoc::smoc_event& dataAvailableEvent()
    { return dae; }

#ifdef SYSTEMOC_ENABLE_VPC
  void produce(smoc::Detail::PortOutBaseIf *who, const smoc::smoc_ref_event_p &latEvent);
#endif
  void produce(smoc::Detail::PortOutBaseIf *who);

  
  /// @brief See smoc_port_registry
  smoc::Detail::PortOutBaseIf* createEntry();

  /// @brief See smoc_port_registry
  smoc::Detail::PortInBaseIf* createOutlet();

  /*void setChannelID( std::string sourceActor,
                             CoSupport::SystemC::ChannelId id,
                             std::string name ){
    this->value.setChannelID(sourceActor, id, name);
  }*/

private:
  /// @brief The tokenId of the next commit token
  size_t tokenId;

  smoc::smoc_event sae;
  smoc::smoc_event dae;

  typedef std::set<smoc_root_chan*> ChanSet;
  typedef std::set<smoc_root_node*> NodeSet;

  ChanSet chans;
  NodeSet nodes;

  // disabled
  smoc_reset_chan( const this_type & );
  this_type& operator = ( const this_type & );
};


class smoc_reset_chan;


class smoc_reset_outlet
  : public smoc_port_in_if<void,::smoc_1d_port_access_if>,
    public smoc_1d_port_access_if<void>
{
public:
  typedef void              data_type;
  typedef smoc_reset_outlet this_type;
  typedef access_in_type    ring_in_type;
  
  /// @brief Constructor
  smoc_reset_outlet(smoc_reset_chan& chan)
    : chan(chan)
  {}
  
  /// @brief See PortInBaseIf
#ifdef SYSTEMOC_ENABLE_VPC
  void commitRead(size_t n, smoc::Detail::VpcInterface vpcIf)
    { assert(0); }
#endif
  void commitRead(size_t n)
    { assert(0); }

 
  /// @brief See PortInBaseIf
  smoc::smoc_event& dataAvailableEvent(size_t n) {
    assert(n == 1);
    return chan.dataAvailableEvent();
  }

  /// @brief See PortInBaseIf
  size_t numAvailable() const
    { return 1; }

  std::string getChannelName() const
    { return chan.name();}

  /// @brief See PortInBaseIf
  size_t inTokenId() const
    { return chan.inTokenId(); }
  
  /// @brief See smoc_port_in_if
  ring_in_type* getReadPortAccess()
    { return this; }
  
  /// @brief See smoc_1d_port_access_if
  bool tokenIsValid(size_t i) const
    { return true; }

  /// @brief See smoc_1d_port_access_if
#ifdef SYSTEMOC_ENABLE_DEBUG
  /// @brief See smoc_1d_port_access_if
  void setLimit(size_t l) {}
#endif // SYSTEMOC_ENABLE_DEBUG

private:
  smoc_reset_chan& chan;
};

class smoc_reset_entry
  : public smoc_port_out_if<void,::smoc_1d_port_access_if>,
    public smoc_1d_port_access_if<void>
{
public:
  typedef void              data_type;
  typedef smoc_reset_entry  this_type;
  typedef access_out_type   ring_out_type;

  /// @brief Constructor
  smoc_reset_entry(smoc_reset_chan& chan)
    : chan(chan)
  {}
  
  /// @brief See PortOutBaseIf
#ifdef SYSTEMOC_ENABLE_VPC
  void commitWrite(size_t n, smoc::Detail::VpcInterface vpcIf)
    { assert(n == 1); chan.produce(this, vpcIf.getTaskLatEvent()); }
#endif
  void commitWrite(size_t n)
    { assert(n == 1); chan.produce(this); }


  /// @brief See PortOutBaseIf
  smoc::smoc_event &spaceAvailableEvent(size_t n) {
    assert(n == 1);
    return chan.spaceAvailableEvent();
  }

  /// @brief See PortOutBaseIf
  size_t numFree() const
    { return 1; }

  std::string getChannelName() const
    { return chan.name();}

  /// @brief See PortOutBaseIf
  size_t outTokenId() const
    { return chan.outTokenId(); }
  
  /// @brief See smoc_port_out_if
  ring_out_type* getWritePortAccess()
    { return this; }

  /// @brief See smoc_1d_port_access_if
  bool tokenIsValid(size_t i) const
    { return true; }

#ifdef SYSTEMOC_ENABLE_DEBUG
  /// @brief See smoc_1d_port_access_if
  void setLimit(size_t l) {}
#endif // SYSTEMOC_ENABLE_DEBUG

private:
  smoc_reset_chan& chan;
};

class smoc_reset_net
  : public smoc_reset_chan::chan_init,
    protected smoc::Detail::ConnectProvider<
      smoc_reset_net,
      smoc_reset_chan> {
public:
  //typedef void          data_type;
  typedef smoc_reset_net       this_type;
  typedef this_type::chan_type chan_type;
  typedef chan_type::chan_init base_type;
  friend class smoc::Detail::ConnectProvider<this_type,chan_type>;
private:
  chan_type *chan;
public:
  smoc_reset_net()
    : base_type(""), chan(NULL)
  {}

  explicit smoc_reset_net( const std::string& name )
    : base_type(name), chan(NULL)
  {}

  /// @brief Constructor
  smoc_reset_net(const this_type &x)
    : base_type(x), chan(NULL)
  {
    if(x.chan)
      assert(!"Can't copy initializer: Channel already created!");
  }

//using this_type::con_type::operator<<;
  using smoc::Detail::ConnectProvider<
      smoc_reset_net,
      smoc_reset_chan>::connect;

  this_type& operator<<(smoc_reset_port& p)
    { return this_type::con_type::connect(p); }
  
  this_type& connect(smoc_reset_port& p)
    { return this_type::con_type::connect(p); }
  
  this_type& connect(smoc_root_node& n) {
    sassert(getChan()->nodes.insert(&n).second);
    return *this;
  }

  this_type& operator<<(smoc_root_node& n)
    { return connect(n); }

  template<class T>
  this_type& connect(smoc_fifo<T>& f) {
    sassert(getChan()->chans.insert(f.getChan()).second);
    return *this;
  }

  template<class T>
  this_type& operator<<(smoc_fifo<T>& f)
    { return connect(f); }

  template<class T>
  this_type& connect(smoc_multireader_fifo<T>& f) {
    sassert(getChan()->chans.insert(f.getChan()).second);
    return *this;
  }

  template<class T>
  this_type& operator<<(smoc_multireader_fifo<T>& f)
    { return connect(f); }
  
  template<class T, class A>
  this_type& connect(smoc_multiplex_fifo<T,A>& f) {
    sassert(getChan()->chans.insert(f.getChan()).second);
    return *this;
  }

  template<class T, class A>
  this_type& operator<<(smoc_multiplex_fifo<T,A>& f)
    { return connect(f); }

private:
  chan_type *getChan() {
    if (chan == NULL)
      chan = new chan_type(*this);
    return chan;
  }

  // disable
  this_type &operator =(const this_type &);
};

#endif // _INCLUDED_SMOC_RESET_HPP
