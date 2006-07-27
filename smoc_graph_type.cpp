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
#include <smoc_graph_type.hpp>
// #include <systemc/kernel/sc_object_manager.h>

template <typename T_node_type,
          typename T_chan_kind,
          template <typename T_value_type> class T_chan_init_default>
const smoc_node_list smoc_graph_petri<T_node_type, T_chan_kind, T_chan_init_default>::getNodes() const {
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
    
    if ( node != NULL /*&& !node->is_v1_actor*/ )
      subnodes.push_back(node);
  }
  return subnodes;
}

template <typename T_node_type,
          typename T_chan_kind,
          template <typename T_value_type> class T_chan_init_default>
const smoc_chan_list smoc_graph_petri<T_node_type, T_chan_kind, T_chan_init_default>::getChans() const {
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

template <typename T_node_type,
          typename T_chan_kind,
          template <typename T_value_type> class T_chan_init_default>
void smoc_graph_petri<T_node_type, T_chan_kind, T_chan_init_default>::
finalise() {
  {
    smoc_chan_list chans = getChans();
    
    for ( typename smoc_chan_list::iterator iter = chans.begin();
          iter != chans.end();
          ++iter ) {
      (*iter)->finalise();
    }
  }
  {
    smoc_node_list nodes = getNodes();
    
    for ( typename smoc_node_list::iterator iter = nodes.begin();
          iter != nodes.end();
          ++iter )
      if ( !(*iter)->is_v1_actor )
        (*iter)->finalise();
  }
}

template <typename T_node_type,
          typename T_chan_kind,
          template <typename T_value_type> class T_chan_init_default>
void smoc_graph_petri<T_node_type, T_chan_kind, T_chan_init_default>::
pgAssemble( smoc_modes::PGWriter &pgw, const smoc_root_node *n ) const {
  const sc_module *m = this;
  const smoc_node_list ns  = getNodes();
  const smoc_chan_list cs  = getChans();
  const smoc_port_list ps  = n->getPorts();
  
  pgw << "<problemgraph name=\"" << m->name() << "_pg\" id=\"" << pgw.getId() << "\">" << std::endl;
  {
    pgw.indentUp();
    for ( typename smoc_node_list::const_iterator iter = ns.begin();
          iter != ns.end();
          ++iter )
      (*iter)->assemble(pgw);
    for ( typename smoc_chan_list::const_iterator iter = cs.begin();
          iter != cs.end();
          ++iter )
      (*iter)->assemble(pgw);
    for ( typename smoc_node_list::const_iterator iter = ns.begin();
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

template void smoc_graph_petri<smoc_root_node, smoc_fifo_kind, smoc_fifo>::
  pgAssemble(smoc_modes::PGWriter &, const smoc_root_node *) const;
template void smoc_graph_petri<smoc_root_node, smoc_fifo_kind, smoc_fifo>::
  finalise();
