//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2008 Hardware-Software-CoDesign, University of
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

#ifndef _INCLUDED_SMOC_MULTIREADER_FIFO_HPP
#define _INCLUDED_SMOC_MULTIREADER_FIFO_HPP

#include <CoSupport/commondefs.h>

#include <systemoc/smoc_config.h>

#include "smoc_chan_if.hpp"
#include "smoc_root_chan.hpp"
#include "smoc_storage.hpp"
#include "smoc_chan_adapter.hpp"
#include "smoc_fifo.hpp"
#include "detail/smoc_latency_queues.hpp"
#include "detail/smoc_ring_access.hpp"
#include "detail/EventMapManager.hpp"
#ifdef SYSTEMOC_ENABLE_VPC
# include "detail/Queue4Ptr.hpp"
#else
# include "detail/Queue2Ptr.hpp"
#endif

#include <systemc.h>
#include <vector>
#include <queue>
#include <map>

#include "hscd_tdsim_TraceLog.hpp"

#ifdef SYSTEMOC_ENABLE_VPC
# include <systemcvpc/hscd_vpc_Director.h>
#endif //SYSTEMOC_ENABLE_VPC

size_t fsizeMapper(sc_object* instance, size_t n);

/**
 * The base channel implementation
 */
class smoc_multireader_fifo_chan_base
: public smoc_root_chan,
#ifdef SYSTEMOC_ENABLE_VPC
  public Detail::Queue4Ptr
#else
  public Detail::Queue2Ptr
#endif // SYSTEMOC_ENABLE_VPC
{
public:
  friend class smoc_multireader_fifo_entry_base;
  friend class smoc_multireader_fifo_outlet_base;
#ifdef SYSTEMOC_ENABLE_VPC
  friend class Detail::LatencyQueue<smoc_multireader_fifo_chan_base>::VisibleQueue;
  friend class Detail::LatencyQueue<smoc_multireader_fifo_chan_base>::RequestQueue;
#endif // SYSTEMOC_ENABLE_VPC
  
  /// @brief Channel initializer
  class chan_init {
  public:
    friend class smoc_multireader_fifo_chan_base;
  protected:
    chan_init(const char *name, size_t n)
      : name(name), n(n)
    {}
  private:
    const char *name;
    size_t n;
  };

protected:
  
  /// @brief Constructor
  smoc_multireader_fifo_chan_base(const chan_init& i)
    : smoc_root_chan(i.name),
#ifdef SYSTEMOC_ENABLE_VPC
    Queue4Ptr(fsizeMapper(this, i.n)),
    latencyQueue(this),
#else
    Queue2Ptr(fsizeMapper(this, i.n)),
#endif
    tokenId(0)
  {}

  /// @brief See smoc_root_chan
  virtual void assemble(smoc_modes::PGWriter &pgw) const
  {}

  /// @brief See smoc_root_chan
  void channelAttributes(smoc_modes::PGWriter& pgw) const {
    pgw << "<attribute type=\"size\" value=\"" << depthCount() << "\"/>" << std::endl;
  }

  void latencyExpired(size_t n) {
    vpp(n);
    emmAvailable.increasedCount(visibleCount());
  }

private:
  Detail::EventMapManager emmAvailable;
  Detail::EventMapManager emmFree;
#ifdef SYSTEMOC_ENABLE_VPC
  Detail::LatencyQueue<smoc_multireader_fifo_chan_base> latencyQueue;
#endif

  /// @brief The token id of the next commit token
  size_t tokenId;
};

/**
 * This class implements the base channel in interface
 */
class smoc_multireader_fifo_entry_base
: public virtual smoc_chan_in_base_if
{
public:
protected:
  /// @brief Constructor
  smoc_multireader_fifo_entry_base(smoc_multireader_fifo_chan_base& chan)
    : chan(chan)
  {}

  /// @brief See smoc_chan_in_base_if
#ifdef SYSTEMOC_ENABLE_VPC
  void commitRead(size_t consume, const smoc_ref_event_p&)
#else
  void commitRead(size_t consume)
#endif
  {
#ifdef SYSTEMOC_TRACE
    TraceLog.traceCommExecIn(this, consume);
#endif
    chan.rpp(consume);
    chan.emmAvailable.decreasedCount(chan.visibleCount());
    chan.emmFree.increasedCount(chan.freeCount());
  }
  
  /// @brief See smoc_chan_in_base_if
  smoc_event &dataAvailableEvent(size_t n)
    { return chan.emmAvailable.getEvent(chan.visibleCount(), n); }

  /// @brief See smoc_chan_in_base_if
  size_t numAvailable() const
    { return chan.visibleCount(); }
  
  /// @brief See smoc_chan_in_base_if
  size_t inTokenId() const
    { return chan.tokenId - chan.usedCount(); }

private:
  /// @brief The channel base implementation
  smoc_multireader_fifo_chan_base& chan;
};



/**
 * This class implements the base channel out interface
 */
class smoc_multireader_fifo_outlet_base
: public virtual smoc_chan_out_base_if
{
public:
protected:
  /// @brief Constructor
  smoc_multireader_fifo_outlet_base(smoc_multireader_fifo_chan_base& chan)
    : chan(chan)
  {}

  /// @brief See smoc_chan_out_base_if
#ifdef SYSTEMOC_ENABLE_VPC
  void commitWrite(size_t produce, const smoc_ref_event_p& le)
#else
  void commitWrite(size_t produce)
#endif
  {
#ifdef SYSTEMOC_TRACE
    TraceLog.traceCommExecOut(this, produce);
#endif
    chan.tokenId += produce;
    chan.wpp(produce);
    chan.emmFree.decreasedCount(chan.freeCount());
#ifdef SYSTEMOC_ENABLE_VPC
    // Delayed call of latencyExpired(produce);
    chan.latencyQueue.addEntry(produce, le);
#else
    chan.latencyExpired(produce);
#endif
  }
  
  /// @brief See smoc_chan_out_base_if
  smoc_event &spaceAvailableEvent(size_t n)
    { return chan.emmFree.getEvent(chan.freeCount(), n); }
  
  /// @brief See smoc_chan_out_base_if
  size_t numFree() const
    { return chan.freeCount(); }
  
  /// @brief See smoc_chan_out_base_if
  size_t outTokenId() const
    { return chan.tokenId; }

private:
  /// @brief The channel base implementation
  smoc_multireader_fifo_chan_base& chan;
};

template<class> class smoc_multireader_fifo_chan;

/**
 * This class implements the channel in interface
 */
template<class T>
class smoc_multireader_fifo_entry
: public smoc_multireader_fifo_entry_base,
  public smoc_chan_in_if<T,smoc_channel_access>
{
public:
  typedef smoc_multireader_fifo_entry<T> this_type;
  typedef typename this_type::access_type access_type; 
  typedef smoc_chan_in_if<T,smoc_channel_access> iface_type;

  /// @brief Constructor
  smoc_multireader_fifo_entry(smoc_multireader_fifo_chan<T>& chan)
    : smoc_multireader_fifo_entry_base(chan),
      chan(chan)
  {}

protected:
  /// @brief See smoc_chan_in_if
  access_type* getReadChannelAccess()
    { return chan.getReadChannelAccess(); }

private:
  /// @brief The channel implementation
  smoc_multireader_fifo_chan<T>& chan;
};

/**
 * This class implements the channel out interface
 */
template<class T>
class smoc_multireader_fifo_outlet
: public smoc_multireader_fifo_outlet_base,
  public smoc_chan_out_if<T,smoc_channel_access>
{
public:
  typedef smoc_multireader_fifo_outlet<T> this_type;
  typedef typename this_type::access_type access_type; 
  typedef smoc_chan_out_if<T,smoc_channel_access> iface_type;

  /// @brief Constructor
  smoc_multireader_fifo_outlet(smoc_multireader_fifo_chan<T>& chan)
    : smoc_multireader_fifo_outlet_base(chan),
      chan(chan)
  {}

protected:
  /// @brief See smoc_chan_out_if
  access_type* getWriteChannelAccess()
    { return chan.getWriteChannelAccess(); }

private:
  /// @brief The channel implementation
  smoc_multireader_fifo_chan<T>& chan;
};

/**
 * This class implements the data type specific
 * channel storage operations
 *
 * This is not merged with smoc_multireader_fifo_chan because
 * we need a void specialization (only for _some_
 * methods)
 */
template<class T>
class smoc_multireader_fifo_storage
: public smoc_multireader_fifo_chan_base
{
public:
  typedef T                           data_type;
  typedef smoc_multireader_fifo_entry<data_type>  entry_type;
  typedef smoc_multireader_fifo_outlet<data_type> outlet_type;
  typedef smoc_storage<data_type>     storage_type;

  typedef typename entry_type::access_type  access_in_type;
  typedef typename outlet_type::access_type access_out_type;

  typedef smoc_ring_access<
    storage_type,
    typename access_in_type::return_type> access_in_type_impl;
  typedef smoc_ring_access<
    storage_type,
    typename access_out_type::return_type> access_out_type_impl;

  friend class smoc_multireader_fifo_entry<data_type>;
  friend class smoc_multireader_fifo_outlet<data_type>;

  /// @brief Channel initializer
  class chan_init
    : public smoc_multireader_fifo_chan_base::chan_init
  {
  public:
    friend class smoc_multireader_fifo_storage;
    typedef T add_param_ty;

    void add(const add_param_ty& t)
      { marking.push_back(t); }
  protected:
    chan_init(const char* name, size_t n)
      : smoc_multireader_fifo_chan_base::chan_init(name, n)
    {}
  private:
    std::vector<T> marking;
  };

protected:

  /// @brief Constructor
  smoc_multireader_fifo_storage(const chan_init& i)
    : smoc_multireader_fifo_chan_base(i),
      storage(new storage_type[this->fSize()])
  {
    assert(this->depthCount() >= i.marking.size());
    for(size_t j = 0; j < i.marking.size(); ++j) {
      storage[j].put(i.marking[j]);
    }
    wpp(i.marking.size()); vpp(i.marking.size());
  }

  /// @brief Destructor
  ~smoc_multireader_fifo_storage()
    { delete[] storage; }
  
  /// @brief See smoc_root_chan
  void channelContents(smoc_modes::PGWriter& pgw) const {
    pgw << "<fifo tokenType=\"" << typeid(data_type).name() << "\">" << std::endl;
    {
      //*************************INITIAL TOKENS, ETC...***************************
      pgw.indentUp();
      for(size_t n = 0; n < this->visibleCount(); ++n)
        pgw << "<token value=\"" << storage[n].get() << "\"/>" << std::endl;
      pgw.indentDown();
    }
    pgw << "</fifo>" << std::endl;
  }

  access_in_type* getReadChannelAccess() {
    return new access_in_type_impl(
        storage, this->fSize(), &this->rIndex());
  }
  
  access_out_type* getWriteChannelAccess() {
    return new access_out_type_impl(
        storage, this->fSize(), &this->wIndex());
  }

private:
  storage_type* storage;
};

/**
 * This class implements the data type specific
 * channel storage operations (void specialization)
 */
template<>
class smoc_multireader_fifo_storage<void>
: public smoc_multireader_fifo_chan_base
{
public:
  typedef void                        data_type;
  typedef smoc_multireader_fifo_entry<data_type>  entry_type;
  typedef smoc_multireader_fifo_outlet<data_type> outlet_type;
  typedef smoc_storage<data_type>     storage_type;

  typedef entry_type::access_type  access_in_type;
  typedef outlet_type::access_type access_out_type;

  typedef smoc_ring_access<void,void> access_in_type_impl;
  typedef smoc_ring_access<void,void> access_out_type_impl;

  friend class smoc_multireader_fifo_entry<data_type>;
  friend class smoc_multireader_fifo_outlet<data_type>;
  
  /// @brief Channel initializer
  class chan_init
  : public smoc_multireader_fifo_chan_base::chan_init
  {
  public:
    friend class smoc_multireader_fifo_storage;
    typedef size_t add_param_ty;

    void add(const add_param_ty& t)
      { marking += t; }
  protected:
    chan_init(const char* name, size_t n)
      : smoc_multireader_fifo_chan_base::chan_init(name, n)
    {}
  private:
    size_t marking;
  };

protected:

  /// @brief Constructor
  smoc_multireader_fifo_storage(const chan_init& i)
    : smoc_multireader_fifo_chan_base(i)
  {
    assert(this->depthCount() >= i.marking);
    wpp(i.marking); vpp(i.marking);
  }
  
  /// @brief See smoc_root_chan
  void channelContents(smoc_modes::PGWriter& pgw) const {
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

  access_in_type* getReadChannelAccess()
    { return new access_in_type_impl(); }
  
  access_out_type* getWriteChannelAccess()
    { return new access_out_type_impl(); }
};

/**
 * This class provides interfaces and connect methods
 */
template<class T>
class smoc_multireader_fifo_chan
: public smoc_multireader_fifo_storage<T>
{
public:
  typedef T                           data_type;
  typedef smoc_multireader_fifo_entry<data_type>  entry_type;
  typedef smoc_multireader_fifo_outlet<data_type> outlet_type;

  /// @brief Channel initializer
  typedef typename smoc_multireader_fifo_storage<T>::chan_init chan_init;

  /// @brief Constructor
  smoc_multireader_fifo_chan(const chan_init& i)
    : smoc_multireader_fifo_storage<T>(i),
      entry(*this),
      outlet(*this)
  {}

  /// @brief See smoc_root_chan
  sc_port_list getInputPorts() const
    { return entry.getPorts(); }

  /// @brief See smoc_root_chan
  sc_port_list getOutputPorts() const
    { return outlet.getPorts(); }

  /// @brief Nicer compile time error
  struct No_Channel_Adapter_Found__Please_Use_Other_Interface {};

  template<class IFace,class Init>
  void connect(sc_port<IFace>& p, const Init&) {
  
    using namespace SysteMoC::Detail;

    // we can provide smoc_chan_in_if and smoc_chan_out_if
    // interfaces (via entry and outlet)
    typedef typename entry_type::iface_type   IFaceImpl1;
    typedef typename outlet_type::iface_type  IFaceImpl2;

    // corresponding adapters
    typedef smoc_chan_adapter<IFaceImpl1,IFace> Adapter1;
    typedef smoc_chan_adapter<IFaceImpl2,IFace> Adapter2;

    // constructor objects
    typedef ConstructPMParam<
      Adapter1,
      smoc_multireader_fifo_chan<T>,
      entry_type,
      &smoc_multireader_fifo_chan<T>::entry> Cons1;

    typedef ConstructPMParam<
      Adapter2,
      smoc_multireader_fifo_chan<T>,
      outlet_type,
      &smoc_multireader_fifo_chan<T>::outlet> Cons2;

    // try to get adapter
    typedef
      typename Select<
        Adapter1::isAdapter,
        Cons1,
      typename Select<
        Adapter2::isAdapter,
        Cons2,
      No_Channel_Adapter_Found__Please_Use_Other_Interface
      >::result_type
      >::result_type Op;

    // create adapter and pass it to port
    p(Op::apply(*this));
  }

  template<class Init>
  void connect(smoc_port_out<data_type>& p, const Init&)
    { p(outlet); }

  template<class Init>
  void connect(smoc_port_in<data_type>& p, const Init&)
    { p(entry); }

protected:
private:
  entry_type entry;
  outlet_type outlet;
};

/**
 * This class is the channel initializer for smoc_multireader_fifo_chan
 */
template <typename T>
class smoc_multireader_fifo
: public smoc_multireader_fifo_chan<T>::chan_init 
{
public:
  typedef T                 data_type;
  typedef smoc_multireader_fifo<T>      this_type;
  typedef smoc_multireader_fifo_chan<T> chan_type;

  this_type& operator<<(const typename chan_type::chan_init::add_param_ty& x)
    { add(x); return *this; }

  /// @brief Constructor
  smoc_multireader_fifo(size_t n = 1)
    : smoc_multireader_fifo_chan<T>::chan_init(0, n)
  {}

  /// @brief Constructor
  explicit smoc_multireader_fifo(const char* name, size_t n = 1)
    : smoc_multireader_fifo_chan<T>::chan_init(name, n)
  {}

private:
  void reset() {};
};

#endif // _INCLUDED_SMOC_MULTIREADER_FIFO_HPP
