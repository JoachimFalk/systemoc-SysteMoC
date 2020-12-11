// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2012 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#include <systemoc/smoc_config.h>

#include <systemoc/smoc_multiplex_fifo.hpp>
#include <smoc/smoc_graph.hpp>

smoc_multiplex_fifo_chan_base::smoc_multiplex_fifo_chan_base(const chan_init &i, size_t tokenSize)
  : ChanBase(i.name, tokenSize)
#ifdef SYSTEMOC_ENABLE_ROUTING
  , smoc::Detail::QueueFRVWPtr(i.n)
#else //!SYSTEMOC_ENABLE_ROUTING
  , smoc::Detail::QueueRWPtr(i.n)
#endif //!SYSTEMOC_ENABLE_ROUTING
  , fifoOutOfOrder(i.m)
{
  assert(fifoOutOfOrder + 1 <= depthCount());
}

void smoc_multiplex_fifo_chan_base::registerVOutlet(const VOutletMap::value_type &entry) {
  sassert(vOutlets.insert(entry).second);
}

void smoc_multiplex_fifo_chan_base::deregisterVOutlet(FifoId fifoId) {
  vOutlets.erase(fifoId);
}

void smoc_multiplex_fifo_chan_base::registerVEntry(const VEntryMap::value_type &entry) {
  sassert(vEntries.insert(entry).second);
}

void smoc_multiplex_fifo_chan_base::deregisterVEntry(FifoId fifoId) {
  vEntries.erase(fifoId);
}
