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

#ifndef _INCLUDED_SMOC_FIFO_HPP
#define _INCLUDED_SMOC_FIFO_HPP

#include <systemc.h>
#include <vector>
#include <queue>
#include <map>

#include <CoSupport/commondefs.h>
#include <sgx.hpp>

#include <systemoc/smoc_config.h>

#ifdef SYSTEMOC_ENABLE_VPC
# include <systemcvpc/hscd_vpc_Director.h>
#endif //SYSTEMOC_ENABLE_VPC

#include "detail/hscd_tdsim_TraceLog.hpp"

#include "detail/smoc_chan_if.hpp"
#include "detail/smoc_root_chan.hpp"
#include "detail/smoc_storage.hpp"
#include "smoc_chan_adapter.hpp"
#include "detail/smoc_latency_queues.hpp"
#include "detail/smoc_fifo_storage.hpp"
#include "detail/ConnectProvider.hpp"
#include "detail/EventMapManager.hpp"
#ifdef SYSTEMOC_ENABLE_VPC
# include "detail/QueueFRVWPtr.hpp"
#else
# include "detail/QueueRWPtr.hpp"
#endif

#include <smoc/detail/DumpingInterfaces.hpp>

size_t fsizeMapper(sc_object* instance, size_t n);

/**
 * The base channel implementation
 */
class smoc_fifo_chan_base
: public smoc_nonconflicting_chan,
#ifdef SYSTEMOC_ENABLE_VPC
  public Detail::QueueFRVWPtr
#else
  public Detail::QueueRWPtr
#endif // SYSTEMOC_ENABLE_VPC
{
  typedef smoc_fifo_chan_base this_type;

  template<class> friend class smoc_fifo_outlet;
  template<class> friend class smoc_fifo_entry;
public:
  /// @brief Channel initializer
  class chan_init {
  public:
    friend class smoc_fifo_chan_base;
  protected:
    chan_init(const std::string& name, size_t n)
      : name(name), n(n)
    {}
  private:
    std::string name;
    size_t n;
  };

protected:
  
  /// @brief Constructor
  smoc_fifo_chan_base(const chan_init &i);

  /// @brief Detail::LatencyQueue callback
  void latencyExpired(size_t n) {
    vpp(n);
    emmAvailable.increasedCount(visibleCount());
  }

  /// @brief Detail::DIIQueue callback
  void diiExpired(size_t n) {
    fpp(n);
    emmFree.increasedCount(freeCount());
  }

public:
#ifdef SYSTEMOC_ENABLE_SGX
  // FIXME: This should be protected for the SysteMoC user but accessible
  // for SysteMoC visitors
  virtual void dumpInitalTokens(SysteMoC::Detail::IfDumpingInitialTokens *it) = 0;
#endif // SYSTEMOC_ENABLE_SGX
private:
  Detail::EventMapManager emmAvailable;
  Detail::EventMapManager emmFree;
#ifdef SYSTEMOC_ENABLE_VPC
  Detail::LatencyQueue  latencyQueue;
  Detail::DIIQueue      diiQueue;
#endif

  /// @brief The token id of the next commit token
  size_t tokenId;
};

template<class> class smoc_fifo_chan;

/**
 * This class implements the channel in interface
 */
template<class T>
class smoc_fifo_outlet
: public smoc_port_in_if<T,smoc_1d_port_access_if>
{
public:
  typedef smoc_fifo_outlet<T> this_type;
  typedef typename this_type::access_type access_type; 
  typedef smoc_port_in_if<T,smoc_1d_port_access_if> iface_type;

  /// @brief Constructor
  smoc_fifo_outlet(smoc_fifo_chan<T>& chan)
    : chan(chan)
  {}

protected:
  /// @brief See smoc_port_in_base_if
#ifdef SYSTEMOC_ENABLE_VPC
  void commitRead(size_t consume, const smoc_ref_event_p &diiEvent)
#else
  void commitRead(size_t consume)
#endif
  {
#ifdef SYSTEMOC_TRACE
    TraceLog.traceCommExecIn(&chan, consume);
#endif
    chan.rpp(consume);
    chan.emmAvailable.decreasedCount(chan.visibleCount());
#ifdef SYSTEMOC_ENABLE_VPC
    // Delayed call of diiExpired(consume);
    chan.diiQueue.addEntry(consume, diiEvent);
#else
    chan.diiExpired(consume);
#endif
  }
  
  /// @brief See smoc_port_in_base_if
  smoc_event &dataAvailableEvent(size_t n)
    { return chan.emmAvailable.getEvent(chan.visibleCount(), n); }

  /// @brief See smoc_port_in_base_if
  size_t numAvailable() const
    { return chan.visibleCount(); }
  
  /// @brief See smoc_port_in_base_if
  size_t inTokenId() const
    { return chan.tokenId - chan.usedCount(); }
  
  /// @brief See smoc_port_in_if
  access_type* getReadPortAccess()
    { return chan.getReadPortAccess(); }

private:
  /// @brief The channel implementation
  smoc_fifo_chan<T>& chan;
};

/**
 * This class implements the channel out interface
 */
template<class T>
class smoc_fifo_entry
: public smoc_port_out_if<T,smoc_1d_port_access_if>
{
public:
  typedef smoc_fifo_entry<T> this_type;
  typedef typename this_type::access_type access_type; 
  typedef smoc_port_out_if<T,smoc_1d_port_access_if> iface_type;

  /// @brief Constructor
  smoc_fifo_entry(smoc_fifo_chan<T>& chan)
    : chan(chan)
  {}

protected:
  /// @brief See smoc_port_out_base_if
#ifdef SYSTEMOC_ENABLE_VPC
  void commitWrite(size_t produce, const smoc_ref_event_p &latEvent)
#else
  void commitWrite(size_t produce)
#endif
  {
#ifdef SYSTEMOC_TRACE
    TraceLog.traceCommExecOut(&chan, produce);
#endif
    chan.tokenId += produce;
    chan.wpp(produce);
    chan.emmFree.decreasedCount(chan.freeCount());
#ifdef SYSTEMOC_ENABLE_VPC
    // Delayed call of latencyExpired(produce);
    chan.latencyQueue.addEntry(produce, latEvent);
#else
    chan.latencyExpired(produce);
#endif
  }
  
  /// @brief See smoc_port_out_base_if
  smoc_event &spaceAvailableEvent(size_t n)
    { return chan.emmFree.getEvent(chan.freeCount(), n); }
  
  /// @brief See smoc_port_out_base_if
  size_t numFree() const
    { return chan.freeCount(); }
  
  /// @brief See smoc_port_out_base_if
  size_t outTokenId() const
    { return chan.tokenId; }
  
  /// @brief See smoc_port_out_if
  access_type* getWritePortAccess()
    { return chan.getWritePortAccess(); }

private:
  /// @brief The channel implementation
  smoc_fifo_chan<T>& chan;
};

/**
 * This class provides interfaces and connect methods
 */
template<class T>
class smoc_fifo_chan
: public smoc_fifo_storage<T, smoc_fifo_chan_base>
{
  friend class smoc_fifo_outlet<T>;
  friend class smoc_fifo_entry<T>;
public:
  typedef T                           data_type;
  typedef smoc_fifo_chan<data_type>   this_type;
  typedef smoc_fifo_entry<data_type>  entry_type;
  typedef smoc_fifo_outlet<data_type> outlet_type;
  
  /// @brief Channel initializer
  typedef typename smoc_fifo_storage<T, smoc_fifo_chan_base>::chan_init chan_init;

  /// @brief Constructor
  smoc_fifo_chan(const chan_init &i)
    : smoc_fifo_storage<T, smoc_fifo_chan_base>(i)
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
 * This class is the channel initializer for smoc_fifo_chan
 */
template <typename T>
class smoc_fifo
: public smoc_fifo_chan<T>::chan_init,
  public SysteMoC::Detail::ConnectProvider<
    smoc_fifo<T>,
    smoc_fifo_chan<T> >
{
  friend class SysteMoC::Detail::ConnectProvider<smoc_fifo<T>, smoc_fifo_chan<T> >;
public:
  typedef T                 data_type;
  typedef smoc_fifo<T>      this_type;
  typedef smoc_fifo_chan<T> chan_type;
private:
  chan_type *chan;
public:
  /// @brief Constructor
  smoc_fifo(size_t n = 1)
    : smoc_fifo_chan<T>::chan_init("", n), chan(NULL)
  {}

  /// @brief Constructor
  explicit smoc_fifo(const std::string& name, size_t n = 1)
    : smoc_fifo_chan<T>::chan_init(name, n), chan(NULL)
  {}

  /// @brief Constructor
  smoc_fifo(const this_type &x)
    : smoc_fifo_chan<T>::chan_init(x), chan(NULL)
  {}

  this_type &operator<<(const typename chan_type::chan_init::add_param_ty &x)
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

#endif // _INCLUDED_SMOC_FIFO_HPP
