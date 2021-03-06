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

#include <systemoc/smoc_multireader_fifo.hpp>
#include <smoc/smoc_graph.hpp>
#include <smoc/detail/TraceLog.hpp>
#include <smoc/detail/DebugOStream.hpp>

#include <systemoc/smoc_config.h>

smoc_multireader_fifo_chan_base::smoc_multireader_fifo_chan_base(const chan_init &i, size_t tokenSize)
  : ChanBase(i.name, tokenSize)
#ifdef SYSTEMOC_ENABLE_ROUTING
  , QueueFRVWPtr(i.n)
#else //!SYSTEMOC_ENABLE_ROUTING
  , QueueRWPtr(i.n)
#endif //!SYSTEMOC_ENABLE_ROUTING
  , tokenId(0)
{}

void smoc_multireader_fifo_chan_base::consume(size_t n)
{
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Medium)) {
    smoc::Detail::outDbg << "<smoc_multireader_fifo_chan_base::consume this=\"" << this << "\">"
           << std::endl << smoc::Detail::Indent::Up;
  }
#endif // SYSTEMOC_ENABLE_DEBUG
#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
  this->getSimCTX()->getDataflowTraceLog()->traceCommExecIn(this, n);
#endif
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Medium)) {
    smoc::Detail::outDbg << "n: " << n << "; #avail: " << visibleCount()
           << "; #free: " << freeCount() << "; size: " << (qfSize()-1) << std::endl;
  }
#endif // SYSTEMOC_ENABLE_DEBUG
  rpp(n);
  emmData.decreasedCountRenotify(visibleCount());
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Medium)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</smoc_multireader_fifo_chan_base::consume>" << std::endl;
  }
#endif // SYSTEMOC_ENABLE_DEBUG
}

void smoc_multireader_fifo_chan_base::produce(size_t n)
{
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Medium)) {
    smoc::Detail::outDbg << "<smoc_multireader_fifo_chan_base::produce this=\"" << this << "\">"
         << std::endl << smoc::Detail::Indent::Up;
  }
#endif // SYSTEMOC_ENABLE_DEBUG
#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
  this->getSimCTX()->getDataflowTraceLog()->traceCommExecOut(this, n);
#endif
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Medium)) {
    smoc::Detail::outDbg << "n: " << n << "; #avail: " << visibleCount()
         << "; #free: " << freeCount() << "; size: " << (qfSize()-1) << std::endl;
  }
#endif // SYSTEMOC_ENABLE_DEBUG
  tokenId += n;
  wpp(n);
  lessSpace(n);
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Medium)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</smoc_multireader_fifo_chan_base::produce>" << std::endl;
  }
#endif // SYSTEMOC_ENABLE_DEBUG
}

void smoc_multireader_fifo_chan_base::latencyExpired(size_t n) {
  vpp(n);
  moreData(n);
}

void smoc_multireader_fifo_chan_base::readConsumeEventExpired(size_t n) {
  fpp(n);
  moreSpace(n);
}

void smoc_multireader_fifo_chan_base::moreData(size_t n) {
  //std::cout << "more data available: " << visibleCount() << std::endl;
  /*for(OutletMap::const_iterator i = getOutlets().begin();
      i != getOutlets().end(); ++i)
  {
    i->first->moreData(n);
  }*/

  emmData.increasedCount(visibleCount());
}

void smoc_multireader_fifo_chan_base::lessData(size_t n) {
  //std::cout << "less data available: " << visibleCount() << std::endl;
  /*for(OutletMap::const_iterator i = getOutlets().begin();
      i != getOutlets().end(); ++i)
  {
    i->first->lessData(n);
  }*/
  
  emmData.decreasedCount(visibleCount());
}

void smoc_multireader_fifo_chan_base::moreSpace(size_t n) {
  //std::cout << "more space available: " << freeCount() << std::endl;
  /*for(EntryMap::const_iterator i = getEntries().begin();
      i != getEntries().end(); ++i)
  {
    i->first->moreSpace(n);
  }*/
  
  emmSpace.increasedCount(freeCount());
}

void smoc_multireader_fifo_chan_base::lessSpace(size_t n) {
  //std::cout << "less space available: " << freeCount() << std::endl;
  /*for(EntryMap::const_iterator i = getEntries().begin();
      i != getEntries().end(); ++i)
  {
    i->first->lessSpace(n);
  }*/

  emmSpace.decreasedCount(freeCount());
}

void smoc_multireader_fifo_chan_base::doReset() {
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Medium)) {
    smoc::Detail::outDbg << "<smoc_multireader_fifo_chan_base::doReset name=\"" << name() << "\">"
         << std::endl << smoc::Detail::Indent::Up;
  }
#endif // SYSTEMOC_ENABLE_DEBUG

  // queue and initial tokens set up by smoc_fifo_storage...
  /*for(EntryMap::const_iterator i = getEntries().begin();
      i != getEntries().end(); ++i)
  {
    i->first->reset();
  }
  for(OutletMap::const_iterator i = getOutlets().begin();
      i != getOutlets().end(); ++i)
  {
    i->first->reset();
  }*/

  emmSpace.reset();
  emmData.reset();

  moreSpace(freeCount());
  moreData(visibleCount());
    
  ChanBase::doReset();

#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Medium)) {
    smoc::Detail::outDbg << "#avail: " << visibleCount() << "; #free: " << freeCount()
         << "; size: " << (qfSize()-1) << std::endl;
  }
#endif // SYSTEMOC_ENABLE_DEBUG

#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Medium)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</smoc_multireader_fifo_chan_base::doReset>" << std::endl;
  }
#endif // SYSTEMOC_ENABLE_DEBUG
}

size_t smoc_multireader_fifo_chan_base::inTokenId() const {
  return tokenId - usedCount();
}

size_t smoc_multireader_fifo_chan_base::outTokenId() const {
  return tokenId;
}
