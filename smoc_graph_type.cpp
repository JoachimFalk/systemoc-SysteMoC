#include <smoc_graph_type.hpp>
// #include <systemc/kernel/sc_object_manager.h>

template <typename T_node_type,
          typename T_chan_kind,
          template <typename T_value_type> class T_chan_init_default>
const smoc_node_list smoc_graph_petri<T_node_type, T_chan_kind, T_chan_init_default>::getNodes() const {
  smoc_node_list subnodes;
  
  for ( sc_pvector<sc_object*>::const_iterator iter = get_child_objects().begin();
        iter != get_child_objects().end();
        ++iter ) {
    smoc_root_node *node = dynamic_cast<smoc_root_node *>(*iter);
    
    if ( node != NULL && !node->is_v1_actor )
      subnodes.push_back(node);
  }
  return subnodes;
}

template <typename T_node_type,
          typename T_chan_kind,
          template <typename T_value_type> class T_chan_init_default>
const smoc_chan_list smoc_graph_petri<T_node_type, T_chan_kind, T_chan_init_default>::getChans() const {
  smoc_chan_list channels;
  
  for ( sc_pvector<sc_object*>::const_iterator iter = get_child_objects().begin();
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
          ++iter )
      (*iter)->hierarchy = this;
  }
  {
    smoc_node_list nodes = getNodes();
    
    for ( typename smoc_node_list::iterator iter = nodes.begin();
          iter != nodes.end();
          ++iter )
      (*iter)->finalise();
  }
}

template <typename T_node_type,
          typename T_chan_kind,
          template <typename T_value_type> class T_chan_init_default>
void smoc_graph_petri<T_node_type, T_chan_kind, T_chan_init_default>::
pgAssemble( smoc_modes::PGWriter &pgw ) const {
  const sc_module *m = this;
  const smoc_node_list ns  = getNodes();
  const smoc_chan_list cs  = getChans();
  
  pgw << "<problemgraph name=\"" << m->name() << "_pg\" id=\"" << pgw.getId() << "\">" << std::endl;
  {
    pgw.indentUp();
    for ( typename smoc_node_list::const_iterator iter = ns.begin();
          iter != ns.end();
          ++iter )
      (*iter)->assemble(pgw);
    for ( typename smoc_chan_list::const_iterator c_iter = cs.begin();
          c_iter != cs.end();
          ++c_iter ) {
      smoc_port_list out = (*c_iter)->getOutputPorts();
      smoc_port_list in  = (*c_iter)->getInputPorts();
      
      for ( smoc_port_list::const_iterator ps_iter = out.begin();
            ps_iter != out.end();
            ++ps_iter ) {
        for ( smoc_port_list::const_iterator pd_iter = in.begin();
              pd_iter != in.end();
              ++pd_iter ) {
          pgw << "<edge name=\"" << (*c_iter)->name() << "\" "
              << "source=\"" << pgw.getId(*ps_iter) << "\" " 
              << "target=\"" << pgw.getId(*pd_iter) << "\" "
              << "id=\"" << pgw.getId(*c_iter) << "\"/>" << std::endl;
        }
      }
    }
    for ( typename iobind_ty::const_iterator iter = iobind.begin();
          iter != iobind.end();
          ++iter )
      pgw << "<portmapping "
          << "from=\"" << pgw.getId(iter->first) << "\" "
          << "to=\"" << pgw.getId(iter->second) << "\" "
          << "id=\"" << pgw.getId() << "\"/>" << std::endl;
    pgw.indentDown();
  }
  pgw << "</problemgraph>" << std::endl;
}

template <typename T_node_type,
          typename T_chan_kind,
          template <typename T_value_type> class T_chan_init_default>
void smoc_graph_petri<T_node_type, T_chan_kind, T_chan_init_default>::
assemble( smoc_modes::PGWriter &pgw ) const {
  const sc_module *m = this;
  
  if ( iobind.empty() )
    return pgAssemble(pgw);
  else {
    pgw << "<process name=\"" << m->name() << "\" id=\"" << pgw.getId(this) << "\">" << std::endl;
    {
      pgw.indentUp();
      for ( sc_pvector<sc_object*>::const_iterator iter = m->get_child_objects().begin();
            iter != m->get_child_objects().end();
            ++iter ) {
  //      if ( *iter == &fire_port )
  //        continue;
        
        const smoc_root_port *port = dynamic_cast<const smoc_root_port *>(*iter);
        
        if ( !port )
          continue;
        pgw << "<port name=\"" << (*iter)->name() << "\" "
            << "type=\"" << (port->isInput() ? "in" : "out") << "\" "
            << "id=\"" << pgw.getId(port) << "\"/>" << std::endl;
      }
      pgAssemble(pgw);
      pgw.indentDown();
    }
    pgw << "</process>" << std::endl;
  }
}

template void smoc_graph_petri<smoc_root_node, smoc_fifo_kind, smoc_fifo>::assemble
  (smoc_modes::PGWriter&) const;
/*
template void smoc_graph_petri<smoc_choice_node, smoc_fifo_kind, smoc_fifo>::assemble
  (smoc_modes::PGWriter&) const;
template void smoc_graph_petri<smoc_transact_node, smoc_fifo_kind, smoc_fifo>::assemble
  (smoc_modes::PGWriter&) const;
template void smoc_graph_petri<smoc_fixed_transact_node, smoc_fifo_kind, smoc_fifo>::assemble
  (smoc_modes::PGWriter&) const;
template void smoc_graph_petri<smoc_choice_node, smoc_rendezvous_kind, smoc_rendezvous>::assemble
  (smoc_modes::PGWriter&) const;
*/

template void smoc_graph_petri<smoc_root_node, smoc_fifo_kind, smoc_fifo>::
  finalise();
