// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2011 FAU -- Sebastian Graf <sebastian.graf@fau.de>
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

#include <smoc/detail/FifoChanBase.hpp>
//#include <smoc/smoc_graph.hpp>

#include <systemoc/smoc_config.h>

namespace smoc { namespace Detail {

FifoChanBase::FifoChanBase(const chan_init &i, size_t tokenSize)
  : ChanBase(i.name, tokenSize)
#ifdef SYSTEMOC_ENABLE_ROUTING
  , QueueFRVWPtr(i.n)
#else //!SYSTEMOC_ENABLE_ROUTING
  , QueueRWPtr(i.n)
#endif //!SYSTEMOC_ENABLE_ROUTING
  , tokenId(0)
{}

void FifoChanBase::before_end_of_elaboration() {
  ChanBase::before_end_of_elaboration();
  if (getEntries().size() > 1)
    std::cerr << "Error: FIFO " << name() << " has multiple writers!" << std::endl;
  else if (getEntries().size() == 0)
    std::cerr << "Error: FIFO " << name() << " has no writers!" << std::endl;
  if (getOutlets().size() > 1)
    std::cerr << "Error: FIFO " << name() << " has multiple readers!" << std::endl;
  else if (getOutlets().size() == 0)
    std::cerr << "Error: FIFO " << name() << " has no readers!" << std::endl;
  assert(getEntries().size() == 1);
  assert(getOutlets().size() == 1);
}

void FifoChanBase::end_of_simulation() {
  ChanBase::end_of_simulation();
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg
            << this->name() << "\t"
            << this->visibleCount() << "\t"
            << this->qfSize() << "\t"
            << this->freeCount() << "\t"
            << this->usedCount() << std::endl;
  }
#endif // SYSTEMOC_ENABLE_DEBUG
}

void FifoChanBase::doReset() {
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<smoc_fifo_chan_base::doReset name=\"" << name() << "\">"
         << std::endl << smoc::Detail::Indent::Up;
  }
#endif // SYSTEMOC_ENABLE_DEBUG
  ChanBase::doReset();
  // queue and initial tokens set up by smoc_fifo_storage...
  emmSpace.reset();
  emmData.reset();

  emmSpace.increasedCount(freeCount());
  emmData.increasedCount(visibleCount());
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</smoc_fifo_chan_base::doReset>" << std::endl;
  }
#endif // SYSTEMOC_ENABLE_DEBUG
}

} } // namespace smoc::Detail
