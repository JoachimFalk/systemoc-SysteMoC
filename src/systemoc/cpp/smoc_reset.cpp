// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2019 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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
#include <smoc/smoc_graph.hpp>
#include <systemoc/smoc_reset.hpp>
#include <smoc/detail/TraceLog.hpp>
#include <smoc/detail/DebugOStream.hpp>

smoc_reset_chan::chan_init::chan_init(
    const std::string& name)
  : name(name)
{}

smoc_reset_chan::smoc_reset_chan(
    const chan_init &i)
: ChanBase(
#ifndef SYSTEMOC_ENABLE_MAESTROMM_SPEEDUP
    i.name
#endif //!defined(SYSTEMOC_ENABLE_MAESTROMM_SPEEDUP)
  ),
  tokenId(1)
{}
  
smoc::Detail::PortOutBaseIf *smoc_reset_chan::createEntry() {
  return new entry_type(*this);
}

smoc::Detail::PortInBaseIf  *smoc_reset_chan::createOutlet() {
  return new outlet_type(*this);
}

void smoc_reset_chan::doReset() {
  sae.notify();
  ChanBase::doReset();
}

void smoc_reset_chan::produce(smoc::Detail::PortOutBaseIf *who)
{
#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
  this->getSimCTX()->getDataflowTraceLog()->traceCommExecOut(this, 1);
#endif
  tokenId++;

#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Medium)) {
    smoc::Detail::outDbg << "<smoc_reset_chan::reset name=\"" << name() << "\">"
         << std::endl << smoc::Detail::Indent::Up;
  }
#endif // SYSTEMOC_ENABLE_DEBUG

  // reset channels
  for(ChanSet::const_iterator c = chans.begin();
      c != chans.end(); ++c)
  {
    (*c)->doReset();
  }

  // reset nodes
  for(NodeSet::const_iterator n = nodes.begin();
      n != nodes.end(); ++n)
  {
    (*n)->doReset();
  }
  
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Medium)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</smoc_reset_chan::reset>" << std::endl;
  }
#endif // SYSTEMOC_ENABLE_DEBUG
}

