// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2006 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
 *
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

#include <smoc_graph_type.hpp>
// #include <systemc/kernel/sc_object_manager.h>

const smoc_node_list smoc_graph::getNodes() const {
  smoc_node_list subnodes;
  
  for (
#if SYSTEMC_VERSION < 20050714
        sc_pvector<sc_object*>::const_iterator iter = get_child_objects().begin();
#else
        std::vector<sc_object*>::const_iterator iter = get_child_objects().begin();
#endif
        iter != get_child_objects().end();
        ++iter ) {
    smoc_root_node *node = dynamic_cast<smoc_root_node *>(*iter);
    if (node != NULL)
      subnodes.push_back(node);
  }
  return subnodes;
}

const smoc_chan_list smoc_graph::getChans() const {
  smoc_chan_list channels;
  
  for (
#if SYSTEMC_VERSION < 20050714
        sc_pvector<sc_object*>::const_iterator iter = get_child_objects().begin();
#else
        std::vector<sc_object*>::const_iterator iter = get_child_objects().begin();
#endif
        iter != get_child_objects().end();
        ++iter ) {
    smoc_root_chan *chan = dynamic_cast<smoc_root_chan *>(*iter);
    
    if (chan != NULL )
      channels.push_back(chan);
  }
  return channels;
}

void smoc_graph::finalise() {
#ifdef SYSTEMOC_DEBUG
  std::cerr << "smoc_graph::finalise() name == " << name() << std::endl;
#endif
  // finalise for actors must precede finalise for channels,
  // because finalise for channels needs the patched in actor
  // references in the ports which are updated by the finalise
  // methods of their respective actors
  {
    smoc_node_list nodes = getNodes();
    
    for ( smoc_node_list::iterator iter = nodes.begin();
          iter != nodes.end();
          ++iter )
      (*iter)->finalise();
  }
  {
    smoc_chan_list chans = getChans();
    
    for ( smoc_chan_list::iterator iter = chans.begin();
          iter != chans.end();
          ++iter )
      (*iter)->finalise();
  }
  smoc_root_node::finalise();
}

sc_module *smoc_graph::myModule()
  { return this; }

void smoc_graph::pgAssemble(
    smoc_modes::PGWriter &pgw,
    const smoc_root_node *n) const {
  const sc_module *m = this;
  const smoc_node_list ns  = getNodes();
  const smoc_chan_list cs  = getChans();
  const smoc_port_list ps  = n->getPorts();
  
  pgw << "<problemgraph name=\"" << m->name() << "_pg\" id=\"" << pgw.getId() << "\">" << std::endl;
  {
    pgw.indentUp();
    for ( smoc_node_list::const_iterator iter = ns.begin();
          iter != ns.end();
          ++iter )
      (*iter)->assemble(pgw);
    for ( smoc_chan_list::const_iterator iter = cs.begin();
          iter != cs.end();
          ++iter )
      (*iter)->assemble(pgw);
    for ( smoc_node_list::const_iterator iter = ns.begin();
          iter != ns.end();
          ++iter ) {
      smoc_port_list nsps = (*iter)->getPorts();
      
      for ( smoc_port_list::const_iterator ps_iter = nsps.begin();
            ps_iter != nsps.end();
            ++ps_iter ) {
        if ( (*ps_iter)->getParentPort() != NULL ) {
          pgw << "<portmapping "
              << "from=\"" << pgw.getId(*ps_iter) << "\" "
              << "to=\"" << pgw.getId((*ps_iter)->getParentPort()) << "\" "
              << "id=\"" << pgw.getId() << "\"/>" << std::endl;
        }
      }
    }
    pgw.indentDown();
  }
  pgw << "</problemgraph>" << std::endl;
}

void smoc_graph::assembleActor(
    smoc_modes::PGWriter &pgw) const {}
