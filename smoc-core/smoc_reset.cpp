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

#ifdef SYSTEMOC_ENABLE_VPC
# include <systemcvpc/hscd_vpc_Director.h>
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
#else
void smoc_reset_chan::produce(smoc_port_out_base_if *who)
#endif
{
#ifdef SYSTEMOC_TRACE
  TraceLog.traceCommExecOut(this, 1);
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

  // which actor did it?
  smoc_root_node* src = 0;
  for(EntryMap::const_iterator i = getEntries().begin();
      i != getEntries().end(); ++i)
  {
    if(i->first == who) {
      src = dynamic_cast<smoc_root_node*>(i->second->get_parent());
      break;
    }
  }
  assert(src);

  // reset nodes
  for(NodeSet::const_iterator n = nodes.begin();
      n != nodes.end(); ++n)
  {
    if(*n == src) {
      // if node is the reset source, parent scheduler will re-add transitions
      (*n)->doReset();
      //(*n)->notifyReset();
    }
    else {
      smoc_graph_base *graph =
        dynamic_cast<smoc_graph_base*>((*n)->get_parent());
      if(graph) {
        // If we have no parent graph, top scheduler will re-add transitions
        graph->reinitScheduling(*n);
      }
    }
  }
  
#ifdef SYSTEMOC_DEBUG
  outDbg << Indent::Down << "</smoc_reset_chan::reset>" << std::endl;
#endif // SYSTEMOC_DEBUG
}

