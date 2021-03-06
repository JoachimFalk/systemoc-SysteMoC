// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Christian Zebelein <christian.zebelein@fau.de>
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Sebastian Graf <sebastian.graf@fau.de>
 *   2012 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2014 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2015 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Simone Müller <simone.mueller@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2019 FAU -- Joachim Falk <joachim.falk@fau.de>
 * 
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option) any
 * later version.
 * 
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#ifndef _INCLUDED_SYSTEMOC_SMOC_MULTIREADER_FIFO_HPP
#define _INCLUDED_SYSTEMOC_SMOC_MULTIREADER_FIFO_HPP

//#include "smoc_fifo.hpp"
//#include "smoc_chan_adapter.hpp"
#include "detail/smoc_chan_if.hpp"
#include "../smoc/smoc_token_traits.hpp"
#include "../smoc/detail/ChanBase.hpp"
#include "../smoc/detail/ConnectProvider.hpp"
#include "../smoc/detail/DumpingInterfaces.hpp"
#include "../smoc/detail/EventMapManager.hpp"
#include "../smoc/detail/FifoStorage.hpp"
#include "../smoc/detail/QueueFRVWPtr.hpp"
#include "../smoc/detail/QueueRWPtr.hpp"

#include <systemoc/smoc_config.h>

#include <CoSupport/commondefs.h>

#include <systemc>

/**
 * The base channel implementation
 */
class smoc_multireader_fifo_chan_base
: public smoc::Detail::ChanBase,
#ifdef SYSTEMOC_ENABLE_ROUTING
  public smoc::Detail::QueueFRVWPtr
#else //!SYSTEMOC_ENABLE_ROUTING
  public smoc::Detail::QueueRWPtr
#endif // !SYSTEMOC_ENABLE_ROUTING
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
  smoc_multireader_fifo_chan_base(const chan_init &i, size_t tokenSize);

  void doReset();

  /// @brief Called by outlet if it did consume tokens
  void consume(size_t n);
  
  /// @brief Called by entry if it did produce tokens
  void produce(size_t n);
  
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
#ifdef SYSTEMOC_ENABLE_DEBUG
    std::cerr << this->name() << "\t"
              << this->visibleCount() << "\t"
              << this->qfSize() << "\t"
              << this->freeCount() << "\t"
              << this->usedCount() << std::endl;
#endif // SYSTEMOC_ENABLE_DEBUG
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

  /// @brief See PortBaseIf
  void commStart(size_t consume) {
    chan.consume(consume);
  }
  /// @brief See PortBaseIf
  void commFinish(size_t consume, bool dropped = false) {
    assert(!dropped);
    chan.readConsumeEventExpired(consume);
  }

  /// @brief See PortBaseIf
  void commExec(size_t consume) {
    commStart(consume);
    commFinish(consume);
  }

  /// @brief See PortInBaseIf
  smoc::smoc_event &dataAvailableEvent(size_t n)
    { assert(n); return chan.emmData.getEvent(n); }

  /// @brief See PortInBaseIf
  size_t numAvailable() const
    { return chan.visibleCount(); }

  const char *name() const
    { return chan.name(); }
  
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
  /// @brief See PortBaseIf
  void commStart(size_t produce) {
    chan.produce(produce);
  }
  /// @brief See PortBaseIf
  void commFinish(size_t produce, bool dropped = false) {
    assert(!dropped);
    chan.latencyExpired(produce);
  }

  /// @brief See PortBaseIf
  void commExec(size_t produce) {
    commStart(produce);
    commFinish(produce);
  }

  /// @brief See PortOutBaseIf
  smoc::smoc_event &spaceAvailableEvent(size_t n)
    { assert(n); return chan.emmSpace.getEvent(n); }
  
  /// @brief See PortOutBaseIf
  size_t numFree() const
    { return chan.freeCount(); }

  const char *name() const
    { return chan.name(); }

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
: public smoc::Detail::FifoStorage<T, smoc_multireader_fifo_chan_base>
{
  friend class smoc_multireader_fifo_outlet<T>;
  friend class smoc_multireader_fifo_entry<T>;
public:
  typedef T                                       data_type;
  typedef smoc_multireader_fifo_chan<data_type>   this_type;
  typedef smoc_multireader_fifo_entry<data_type>  entry_type;
  typedef smoc_multireader_fifo_outlet<data_type> outlet_type;

  /// @brief Channel initializer
  typedef typename smoc::Detail::FifoStorage<T, smoc_multireader_fifo_chan_base>::chan_init chan_init;

  /// @brief Constructor
  smoc_multireader_fifo_chan(const chan_init &i)
    : smoc::Detail::FifoStorage<T, smoc_multireader_fifo_chan_base>(i, smoc::smoc_token_traits<T>::tokenSize())
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
  friend typename this_type::con_type;
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
private:
  chan_type *getChan() {
    if (chan == nullptr)
      chan = new chan_type(*this);
    return chan;
  }

  // disable
  this_type &operator =(const this_type &);
};

#endif /* _INCLUDED_SYSTEMOC_SMOC_MULTIREADER_FIFO_HPP */
