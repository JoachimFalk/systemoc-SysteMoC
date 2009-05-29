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

#include <vector>
#include <queue>
#include <map>

#include <CoSupport/commondefs.h>

#include <boost/noncopyable.hpp>

#include <systemc.h>

#include <systemoc/smoc_config.h>

#include <sgx.hpp>

#ifdef SYSTEMOC_ENABLE_VPC
# include <systemcvpc/hscd_vpc_Director.h>
#endif //SYSTEMOC_ENABLE_VPC

#include "detail/smoc_chan_if.hpp"
#include "detail/smoc_root_chan.hpp"
#include "smoc_storage.hpp"
#include "smoc_chan_adapter.hpp"
#include "smoc_fifo.hpp"
#include "detail/smoc_latency_queues.hpp"
#include "detail/smoc_fifo_storage.hpp"
#include "detail/ConnectProvider.hpp"
#include "detail/EventMapManager.hpp"
#ifdef SYSTEMOC_ENABLE_VPC
# include "detail/QueueFRVWPtr.hpp"
#else
# include "detail/QueueRWPtr.hpp"
#endif
#include "hscd_tdsim_TraceLog.hpp"

size_t fsizeMapper(sc_object* instance, size_t n);

class smoc_multireader_scheduler {
public:
  friend class smoc_multireader_fifo_chan_base;

  virtual ~smoc_multireader_scheduler()
    {}

protected:
};

/**
 * The base channel implementation
 */
class smoc_multireader_fifo_chan_base
: public smoc_root_chan,
#ifdef SYSTEMOC_ENABLE_VPC
  public Detail::QueueFRVWPtr
#else
  public Detail::QueueRWPtr
#endif // SYSTEMOC_ENABLE_VPC
{
  typedef smoc_multireader_fifo_chan_base this_type;
public:
  template<class> friend class smoc_multireader_fifo_outlet;
  template<class> friend class smoc_multireader_fifo_entry;
  
  /// @brief Channel initializer
  class chan_init {
  public:
    friend class smoc_multireader_fifo_chan_base;
  protected:
    chan_init(const std::string& name, size_t n, smoc_multireader_scheduler* so)
      : name(name), n(n), so(so)
    {}
  private:
    std::string name;
    size_t n;
    smoc_multireader_scheduler* so;
  };

protected:
  /// @brief Constructor
  smoc_multireader_fifo_chan_base(const chan_init &i);

  /// @brief SystemC callback
  void start_of_simulation();

  /// @brief Called by outlet if it did consume tokens
#ifdef SYSTEMOC_ENABLE_VPC
  void consume(size_t n, const smoc_ref_event_p &diiEvent);
#else
  void consume(size_t n);
#endif
  
  /// @brief Called by entry if it did produce tokens
#ifdef SYSTEMOC_ENABLE_VPC
  void produce(size_t n, const smoc_ref_event_p &latEvent);
#else
  void produce(size_t n);
#endif
  
  /// @brief Calculate token id for next consumed token
  size_t inTokenId() const;

  /// @brief Calculate token id for next produced token
  size_t outTokenId() const;

  /// @brief Available token count
  size_t numAvailable() const;

  /// @brief Available free space
  size_t numFree() const;

#ifdef SYSTEMOC_ENABLE_SGX
  SystemCoDesigner::SGX::Fifo::Ptr fifo;
#endif

private:
#ifdef SYSTEMOC_ENABLE_VPC
  Detail::LatencyQueue  latencyQueue;
  Detail::DIIQueue      diiQueue;
#endif

  /// @brief The token id of the next produced token
  size_t tokenId;

  /// @brief The default scheduler
  smoc_multireader_scheduler schedDefault;

  /// @brief The scheduler used for outlets
  smoc_multireader_scheduler* schedOutlets;
  
  /// @brief Detail::LatencyQueue callback
  void latencyExpired(size_t n);

  /// @brief Detail::DIIQueue callback
  void diiExpired(size_t n);
  
  /// @brief Called by outlets when more data is available
  void moreData(size_t n);
  
  /// @brief Called by outlets when less data is available
  void lessData(size_t n);
  
  /// @brief Called by entries when more space is available
  void moreSpace(size_t n);
  
  /// @brief Called by entries when less space is available
  void lessSpace(size_t n);

#ifdef SYSTEMOC_ENABLE_SGX
  void assembleXML();
#endif
};

template<class> class smoc_multireader_fifo_chan;

/**
 * This class implements the channel in interface
 * to be connected to a smoc_port_in<T>. From
 * the fifo perspective this is a class provides
 * data to an input port, therefore is an outlet.
 */
template<class T>
class smoc_multireader_fifo_outlet
: public smoc_port_in_if<T,smoc_1d_port_access_if>
{
public:
  typedef smoc_multireader_fifo_outlet<T>           this_type;
  typedef typename this_type::access_type           access_type; 
  typedef smoc_port_in_if<T,smoc_1d_port_access_if> iface_type;

  /// @brief Constructor
  smoc_multireader_fifo_outlet(smoc_multireader_fifo_chan<T>& chan)
    : chan(chan)
    {}

protected:
  /// @brief See smoc_port_in_base_if
#ifdef SYSTEMOC_ENABLE_VPC
  void commitRead(size_t consume, const smoc_ref_event_p &diiEvent)
    { chan.consume(consume, diiEvent); }
#else
  void commitRead(size_t consume)
    { chan.consume(consume); }
#endif

  /// @brief See smoc_port_in_base_if
  smoc_event &dataAvailableEvent(size_t n)
    { assert(n); return emm.getEvent(0, n); }

  /// @brief See smoc_port_in_base_if
  size_t numAvailable() const
    { return chan.numAvailable(); }
  
  /// @brief See smoc_port_in_base_if
  size_t inTokenId() const
    { return chan.inTokenId(); }
  
  /// @brief See smoc_port_in_base_if
  void moreData()
    { emm.increasedCount(numAvailable()); }

  /// @brief See smoc_port_in_base_if
  void lessData()
    { emm.decreasedCount(numAvailable()); }

  /// @brief See smoc_port_in_if
  access_type* getReadPortAccess()
    { return chan.getReadPortAccess(); }

private:
  /// @brief The channel implementation
  smoc_multireader_fifo_chan<T>& chan;
  
  /// @brief Private event manager
  Detail::EventMapManager emm;
};

/**
 * This class implements the channel out interface
 * to be connected to a smoc_port_out<T>. From
 * the fifo perspective this is a class receives
 * data from an output port, therefore is an entry.
 */
template<class T>
class smoc_multireader_fifo_entry
: public smoc_port_out_if<T,smoc_1d_port_access_if>
{
public:
  typedef smoc_multireader_fifo_entry<T>              this_type;
  typedef typename this_type::access_type             access_type; 
  typedef smoc_port_out_if<T,smoc_1d_port_access_if>  iface_type;

  /// @brief Constructor
  smoc_multireader_fifo_entry(smoc_multireader_fifo_chan<T>& chan)
    : chan(chan)
    {}

protected:
  /// @brief See smoc_port_out_base_if
#ifdef SYSTEMOC_ENABLE_VPC
  void commitWrite(size_t produce, const smoc_ref_event_p &latEvent)
    { chan.produce(produce, latEvent); }
#else
  void commitWrite(size_t produce)
    { chan.produce(produce); }
#endif

  /// @brief See smoc_port_out_base_if
  smoc_event &spaceAvailableEvent(size_t n)
    { assert(n); return emm.getEvent(0, n); }
  
  /// @brief See smoc_port_out_base_if
  size_t numFree() const
    { return chan.numFree(); }
  
  /// @brief See smoc_port_out_base_if
  size_t outTokenId() const
    { return chan.outTokenId(); }
  
  /// @brief See smoc_port_in_base_if
  void moreSpace()
    { emm.increasedCount(numFree()); }

  /// @brief See smoc_port_in_base_if
  void lessSpace()
    { emm.decreasedCount(numFree()); }

  /// @brief See smoc_port_out_if
  access_type* getWritePortAccess()
    { return chan.getWritePortAccess(); }

private:
  /// @brief The channel implementation
  smoc_multireader_fifo_chan<T>& chan;
  
  /// @brief Private event manager
  Detail::EventMapManager emm;
};

/**
 * This class provides interfaces and connect methods
 */
template<class T>
class smoc_multireader_fifo_chan
: public smoc_fifo_storage<T, smoc_multireader_fifo_chan_base>
{
  friend class smoc_multireader_fifo_outlet<T>;
  friend class smoc_multireader_fifo_entry<T>;
public:
  typedef T                                       data_type;
  typedef smoc_multireader_fifo_chan<data_type>   this_type;
  typedef smoc_multireader_fifo_entry<data_type>  entry_type;
  typedef smoc_multireader_fifo_outlet<data_type> outlet_type;

  /// @brief Channel initializer
  typedef typename smoc_fifo_storage<T, smoc_multireader_fifo_chan_base>::chan_init chan_init;

  /// @brief Constructor
  smoc_multireader_fifo_chan(const chan_init &i)
    : smoc_fifo_storage<T, smoc_multireader_fifo_chan_base>(i)
  {}
protected:
  /// @brief See smoc_port_registry
  smoc_port_out_base_if* createEntry()
    { return new entry_type(*this); }

  /// @brief See smoc_port_registry
  smoc_port_in_base_if* createOutlet()
    { return new outlet_type(*this); }

private:
};

/**
 * This class is the channel initializer for smoc_multireader_fifo_chan
 */
template <typename T>
class smoc_multireader_fifo
: public smoc_multireader_fifo_chan<T>::chan_init,
  public SysteMoC::Detail::ConnectProvider<
    smoc_multireader_fifo<T>,
    smoc_multireader_fifo_chan<T> > {
  typedef smoc_multireader_fifo<T> this_type;

  friend class SysteMoC::Detail::ConnectProvider<this_type, typename this_type::chan_type>;
public:
  typedef T                             data_type;
  typedef typename this_type::chan_type chan_type;
private:
  chan_type *chan;
public:
  /// @brief Constructor
  smoc_multireader_fifo(size_t n = 1, smoc_multireader_scheduler* so = 0)
    : smoc_multireader_fifo_chan<T>::chan_init("", n, so), chan(NULL)
  {}

  /// @brief Constructor
  explicit smoc_multireader_fifo(
      const std::string& name, size_t n = 1, smoc_multireader_scheduler* so = 0)
    : smoc_multireader_fifo_chan<T>::chan_init(name, n, so), chan(NULL)
  {}

  /// @brief Constructor
  smoc_multireader_fifo(const this_type &x)
    : smoc_multireader_fifo_chan<T>::chan_init(x), chan(NULL)
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

#endif // _INCLUDED_SMOC_MULTIREADER_FIFO_HPP
