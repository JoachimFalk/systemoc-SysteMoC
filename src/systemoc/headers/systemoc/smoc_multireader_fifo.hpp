//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2017 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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

#include <CoSupport/compatibility-glue/nullptr.h>

#include <CoSupport/commondefs.h>

#include <boost/noncopyable.hpp>

#include <systemc>

#include <systemoc/smoc_config.h>

#ifdef SYSTEMOC_ENABLE_VPC
# include <vpc.hpp>
#endif //SYSTEMOC_ENABLE_VPC

#include "detail/smoc_chan_if.hpp"
#include <smoc/detail/ChanBase.hpp>
//#include "../smoc/detail/Storage.hpp"
#include "smoc_chan_adapter.hpp"
#include "smoc_fifo.hpp"
#include "detail/smoc_fifo_storage.hpp"
#include <smoc/detail/ConnectProvider.hpp>
#include <smoc/detail/EventMapManager.hpp>
#ifdef SYSTEMOC_ENABLE_VPC
# include <smoc/detail/LatencyQueue.hpp>
# include <smoc/detail/EventQueue.hpp>
# include <smoc/detail/QueueFRVWPtr.hpp>
#else
# include <smoc/detail/QueueRWPtr.hpp>
#endif

#include <smoc/detail/DumpingInterfaces.hpp>

/**
 * The base channel implementation
 */
class smoc_multireader_fifo_chan_base
: public smoc::Detail::ChanBase,
#ifdef SYSTEMOC_ENABLE_VPC
  public smoc::Detail::QueueFRVWPtr
#else
  public smoc::Detail::QueueRWPtr
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
    chan_init(const std::string& name, size_t n)
      : name(name), n(n)
    {}
  private:
    std::string name;
    size_t n;
  };

protected:
  /// @brief Constructor
  smoc_multireader_fifo_chan_base(const chan_init &i);

  void doReset();

  /// @brief Called by outlet if it did consume tokens
#ifdef SYSTEMOC_ENABLE_VPC
  void consume(smoc::Detail::PortInBaseIf *who, size_t n, smoc::smoc_vpc_event_p const &readConsumeEvent);
#else
  void consume(smoc::Detail::PortInBaseIf *who, size_t n);
#endif
  
  /// @brief Called by entry if it did produce tokens
#ifdef SYSTEMOC_ENABLE_VPC
  void produce(size_t n, smoc::Detail::VpcInterface vpcIf);
#else
  void produce(size_t n);
#endif
  
  /// @brief Calculate token id for next consumed token
  size_t inTokenId() const;

  /// @brief Calculate token id for next produced token
  size_t outTokenId() const;

public:
#ifdef SYSTEMOC_ENABLE_SGX
  // FIXME: This should be protected for the SysteMoC user but accessible
  // for SysteMoC visitors
  virtual void dumpInitialTokens(smoc::Detail::IfDumpingInitialTokens *it) = 0;
#endif // SYSTEMOC_ENABLE_SGX
private:
#ifdef SYSTEMOC_ENABLE_VPC
  smoc::Detail::LatencyQueue        latencyQueue;
  smoc::Detail::EventQueue<size_t>  readConsumeQueue;
#endif
  
  smoc::Detail::EventMapManager emmData;
  smoc::Detail::EventMapManager emmSpace;

  /// @brief The token id of the next produced token
  size_t tokenId;

  /// @brief callback for latencyQueue
  void latencyExpired(size_t n);

  /// @brief callback for readConsumeQueue
  void readConsumeEventExpired(size_t n);
  
  /// @brief Called when more data is available
  void moreData(size_t n);
  
  /// @brief Called when less data is available
  void lessData(size_t n);
  
  /// @brief Called when more space is available
  void moreSpace(size_t n);
  
  /// @brief Called when less space is available
  void lessSpace(size_t n);

public:
  virtual void end_of_simulation(){
#ifdef SYSTEMOC_DEBUG
    std::cerr << this->name() << "\t"
              << this->visibleCount() << "\t"
              << this->qfSize() << "\t"
              << this->freeCount() << "\t"
              << this->usedCount() << std::endl;
# ifdef SYSTEMOC_ENABLE_VPC
    latencyQueue.dump();
    readConsumeQueue.dump();
# endif // SYSTEMOC_ENABLE_VPC
#endif // SYSTEMOC_DEBUG
  }

};

template<class> class smoc_multireader_fifo_chan;

/**
 * This class implements the channel in interface
 * to be connected to a smoc_port_in<T>. From
 * the fifo perspective this is a class provides
 * data to an input port, therefore is an outlet.
 */
template<class T>
class smoc_multireader_fifo_outlet: public smoc_port_in_if<T> {
  typedef smoc_multireader_fifo_outlet<T>  this_type;
public:
  typedef smoc_port_in_if<T>               iface_type;
  typedef typename iface_type::access_type access_type;

  /// @brief Constructor
  smoc_multireader_fifo_outlet(smoc_multireader_fifo_chan<T> &chan)
    : chan(chan)
    {}

protected:
  /// @brief See PortInBaseIf
#ifdef SYSTEMOC_ENABLE_VPC
  void commitRead(size_t n, smoc::smoc_vpc_event_p const &readConsumeEvent)
    { chan.consume(this, n, readConsumeEvent); }
#else
  void commitRead(size_t n)
    { chan.consume(this, n); }
#endif

  /// @brief See PortInBaseIf
  smoc::smoc_event &dataAvailableEvent(size_t n)
    { assert(n); return chan.emmData.getEvent(n); }

  /// @brief See PortInBaseIf
  size_t numAvailable() const
    { return chan.visibleCount(); }

  std::string getChannelName() const
    { return chan.name();}
  
  /// @brief See PortInBaseIf
  size_t inTokenId() const
    { return chan.inTokenId(); }
  
  /// @brief See PortInBaseIf
  void moreData(size_t)
    { /*emm.increasedCount(chan.visibleCount());*/ }

  /// @brief See PortInBaseIf
  void lessData(size_t)
    { /*emm.decreasedCount(chan.visibleCount());*/ }

  /// @brief See PortInBaseIf
  void reset()
    { /*emm.reset();*/ }

  /// @brief See smoc_port_in_if
  access_type* getReadPortAccess()
    { return chan.getReadPortAccess(); }

private:
  /// @brief The channel implementation
  smoc_multireader_fifo_chan<T>& chan;
};

/**
 * This class implements the channel out interface
 * to be connected to a smoc_port_out<T>. From
 * the fifo perspective this is a class receives
 * data from an output port, therefore is an entry.
 */
template<class T>
class smoc_multireader_fifo_entry: public smoc_port_out_if<T> {
  typedef smoc_multireader_fifo_entry<T>   this_type;
public:
  typedef smoc_port_out_if<T>              iface_type;
  typedef typename iface_type::access_type access_type;

  /// @brief Constructor
  smoc_multireader_fifo_entry(smoc_multireader_fifo_chan<T> &chan)
    : chan(chan)
    {}

protected:
  /// @brief See PortOutBaseIf
#ifdef SYSTEMOC_ENABLE_VPC
  void commitWrite(size_t n, smoc::Detail::VpcInterface vpcIf)
    { chan.produce(n, vpcIf); }
#else
  void commitWrite(size_t n)
    { chan.produce(n); }
#endif

  /// @brief See PortOutBaseIf
  smoc::smoc_event &spaceAvailableEvent(size_t n)
    { assert(n); return chan.emmSpace.getEvent(n); }
  
  /// @brief See PortOutBaseIf
  size_t numFree() const
    { return chan.freeCount(); }

  std::string getChannelName() const
    { return chan.name();}

  /// @brief See PortOutBaseIf
  size_t outTokenId() const
    { return chan.outTokenId(); }
  
  /// @brief See PortOutBaseIf
  void moreSpace(size_t)
    { /*emm.increasedCount(chan.freeCount());*/ }

  /// @brief See PortOutBaseIf
  void lessSpace(size_t)
    { /*emm.decreasedCount(chan.freeCount());*/ }

  /// @brief See PortOutBaseIf
  void reset()
    { /*emm.reset();*/ }

  /// @brief See smoc_port_out_if
  access_type* getWritePortAccess()
    { return chan.getWritePortAccess(); }

private:
  /// @brief The channel implementation
  smoc_multireader_fifo_chan<T>& chan;
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
  smoc::Detail::PortOutBaseIf *createEntry()
    { return new entry_type(*this); }

  /// @brief See smoc_port_registry
  smoc::Detail::PortInBaseIf  *createOutlet()
    { return new outlet_type(*this); }

private:
};

/**
 * This class is the channel initializer for smoc_multireader_fifo_chan
 */
template <typename T>
class smoc_multireader_fifo
: public smoc_multireader_fifo_chan<T>::chan_init,
  public smoc::Detail::ConnectProvider<
    smoc_multireader_fifo<T>,
    smoc_multireader_fifo_chan<T> > {

public:
  //typedef T                             data_type;
  typedef smoc_multireader_fifo<T>      this_type;
  typedef typename this_type::chan_type chan_type;
  typedef typename chan_type::chan_init base_type;
#ifdef _MSC_VER
  friend typename this_type::con_type;
#else
  friend class this_type::con_type;
#endif // _MSC_VER
  friend class smoc_reset_net;
private:
  chan_type *chan;
public:
  /// @brief Constructor
  smoc_multireader_fifo(size_t n = 1)
    : base_type("", n), chan(nullptr)
  {}

  /// @brief Constructor
  explicit smoc_multireader_fifo(
      const std::string& name, size_t n = 1)
    : base_type(name, n), chan(nullptr)
  {}

  /// @brief Constructor
  smoc_multireader_fifo(const this_type &x)
    : base_type(x), chan(nullptr)
  {
    if(x.chan)
      assert(!"Can't copy initializer: Channel already created!");
  }

  this_type &operator <<(typename this_type::add_param_ty x) {
    if(chan)
      assert(!"Can't place initial token: Channel already created!");
    this->add(x);
    return *this;
  }

////using this_type::con_type::operator<<;
//using smoc::Detail::ConnectProvider<smoc_multireader_fifo<T>, smoc_multireader_fifo_chan<T> >::operator<<;

private:
  chan_type *getChan() {
    if (chan == nullptr)
      chan = new chan_type(*this);
    return chan;
  }

  // disable
  this_type &operator =(const this_type &);
};

#endif // _INCLUDED_SMOC_MULTIREADER_FIFO_HPP
