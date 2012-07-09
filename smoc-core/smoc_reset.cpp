// vim: set sw=2 ts=8:
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <systemoc/smoc_config.h>
#include <systemoc/smoc_graph_type.hpp>
#include <systemoc/smoc_reset.hpp>
#include <systemoc/detail/smoc_debug_stream.hpp>
#include <smoc/detail/TraceLog.hpp>

#ifdef SYSTEMOC_ENABLE_VPC
# include <vpc.hpp>
#endif //SYSTEMOC_ENABLE_VPC

smoc_reset_chan::chan_init::chan_init(
    const std::string& name)
  : name(name)
{}

smoc_reset_chan::smoc_reset_chan(
    const chan_init &i)
  : smoc_multicast_chan(i.name),
    tokenId(1)
{}
  
smoc_port_out_base_if* smoc_reset_chan::createEntry() {
  return new entry_type(*this);
}

smoc_port_in_base_if* smoc_reset_chan::createOutlet() {
  return new outlet_type(*this);
}

void smoc_reset_chan::doReset() {
  sae.notify();
  smoc_root_chan::doReset();
}

#ifdef SYSTEMOC_ENABLE_VPC
void smoc_reset_chan::produce(smoc_port_out_base_if *who, const smoc_ref_event_p &latEvent)
{
#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
  this->getSimCTX()->getDataflowTraceLog()->traceCommExecOut(this, 1);
#endif
  tokenId++;

#ifdef SYSTEMOC_DEBUG
  outDbg << "<smoc_reset_chan::reset name=\"" << name() << "\">"
         << std::endl << Indent::Up;
#endif // SYSTEMOC_DEBUG

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

#ifdef SYSTEMOC_DEBUG
  outDbg << Indent::Down << "</smoc_reset_chan::reset>" << std::endl;
#endif // SYSTEMOC_DEBUG
}
#endif

void smoc_reset_chan::produce(smoc_port_out_base_if *who)
{
#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
  this->getSimCTX()->getDataflowTraceLog()->traceCommExecOut(this, 1);
#endif
  tokenId++;

#ifdef SYSTEMOC_DEBUG
  outDbg << "<smoc_reset_chan::reset name=\"" << name() << "\">"
         << std::endl << Indent::Up;
#endif // SYSTEMOC_DEBUG

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
  
#ifdef SYSTEMOC_DEBUG
  outDbg << Indent::Down << "</smoc_reset_chan::reset>" << std::endl;
#endif // SYSTEMOC_DEBUG
}

