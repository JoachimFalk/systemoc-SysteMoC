// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Christian Zebelein <christian.zebelein@fau.de>
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Sebastian Graf <sebastian.graf@fau.de>
 *   2012 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2012 FAU -- Rafael Rosales <rafael.rosales@fau.de>
 *   2014 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2015 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2015 FAU -- Rafael Rosales <rafael.rosales@fau.de>
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

#ifndef _INCLUDED_SMOC_DETAIL_FIFOCHANBASE_HPP
#define _INCLUDED_SMOC_DETAIL_FIFOCHANBASE_HPP

#include "ChanBase.hpp"
#include "DumpingInterfaces.hpp"
#include "EventMapManager.hpp"

#include "QueueFRVWPtr.hpp"
#include "QueueRWPtr.hpp"

#include <systemoc/smoc_config.h>

#include <CoSupport/commondefs.h>

#include <systemc>

namespace smoc { namespace Detail {

/**
 * The base channel implementation
 */
class FifoChanBase
: public ChanBase,
#ifdef SYSTEMOC_ENABLE_ROUTING
  public QueueFRVWPtr
#else //!SYSTEMOC_ENABLE_ROUTING
  public QueueRWPtr
#endif // !SYSTEMOC_ENABLE_ROUTING
{
  typedef FifoChanBase this_type;

  template<class> friend class FifoOutlet;
  template<class> friend class FifoEntry;
public:
  /// @brief Channel initializer
  class chan_init {
  public:
    friend class FifoChanBase;
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
  FifoChanBase(const chan_init &i);

  /// @brief LatencyQueue callback
  void latencyExpired(size_t n) {
    vpp(n);
    //inform about new data available;
    emmData.increasedCount(visibleCount());
  }

#ifdef SYSTEMOC_ENABLE_ROUTING
  /// @brief LatencyQueue callback #2
  void latencyExpired_dropped(size_t n) {
    invalidateToken(n);
    //inform about new free space;
    emmSpace.increasedCount(freeCount());
  }
#endif //defined(SYSTEMOC_ENABLE_ROUTING)

  /// @brief callback for readConsumeQueue
  void readConsumeEventExpired(size_t n) {
    fpp(n);
    emmSpace.increasedCount(freeCount());
  }

  /// @See smoc_root_chan
  virtual void doReset();

  virtual void before_end_of_elaboration();
  virtual void end_of_simulation();
public:
#ifdef SYSTEMOC_ENABLE_SGX
  // FIXME: This should be protected for the SysteMoC user but accessible
  // for SysteMoC visitors
  virtual void dumpInitialTokens(IfDumpingInitialTokens *it) = 0;

  // This must be overwritten in QueueWithStorageHelper to also resize the
  // storage buffer of the FIFO!
  virtual void resize(size_t n) = 0;
#endif // SYSTEMOC_ENABLE_SGX
private:
  EventMapManager emmData;
  EventMapManager emmSpace;

  /// @brief The token id of the next commit token
  size_t tokenId;

#ifdef SYSTEMOC_ENABLE_ROUTING
 virtual void invalidateToken(size_t n) = 0;
#endif //defined(SYSTEMOC_ENABLE_ROUTING)
};

} } // namespace smoc::Detail

#endif /* _INCLUDED_SMOC_DETAIL_FIFOCHANBASE_HPP */
