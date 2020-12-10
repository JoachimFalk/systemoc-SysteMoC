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

#ifndef _INCLUDED_SMOC_DETAIL_FIFOOUTLET_HPP
#define _INCLUDED_SMOC_DETAIL_FIFOOUTLET_HPP

//#include "smoc_chan_adapter.hpp"
#include "../../systemoc/detail/smoc_chan_if.hpp"

#include <systemoc/smoc_config.h>

#include <CoSupport/commondefs.h>

#include <systemc>

namespace smoc { namespace Detail {

template<class> class FifoChan;

/**
 * This class implements the channel in interface
 */
template<class T>
class FifoOutlet: public smoc_port_in_if<T> {
  typedef FifoOutlet<T>              this_type;
public:
  typedef smoc_port_in_if<T>               iface_type;
  typedef typename iface_type::access_type access_type;

  /// @brief Constructor
  FifoOutlet(FifoChan<T> &chan)
    : chan(chan)
  {}

protected:

  /// @brief See PortBaseIf
  void commStart(size_t consume) {
    chan.rpp(consume);
    chan.emmData.decreasedCount(chan.visibleCount());
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
    { return chan.emmData.getEvent(n); }

  /// @brief See PortInBaseIf
  size_t numAvailable() const
    { return chan.visibleCount(); }

  const char *name() const
    { return chan.name();}

  /// @brief See PortInBaseIf
  size_t inTokenId() const
    { return chan.tokenId - chan.usedCount(); }
  
  /// @brief See smoc_port_in_if
  access_type* getReadPortAccess()
    { return chan.getReadPortAccess(); }

private:
#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
  virtual void traceCommSetup(size_t req){
    this->getSimCTX()->getDataflowTraceLog()->traceCommSetup(&chan, req);
  }
#endif // SYSTEMOC_ENABLE_DATAFLOW_TRACE

  /// @brief The channel implementation
  FifoChan<T> &chan;
};

} } // namespace smoc::Detail

#endif /* _INCLUDED_SMOC_DETAIL_FIFOOUTLET_HPP */
