#include <hscd_graph_type.hpp>
// #include <systemc/kernel/sc_object_manager.h>

template <typename T_node_type,
          typename T_chan_kind,
          template <typename T_value_type> class T_chan_init_default>
void hscd_graph_petri<T_node_type, T_chan_kind, T_chan_init_default>::
pgAssemble( hscd_modes::PGWriter &pgw ) const {
  const sc_module *m = this;
  const nodes_ty ns  = getNodes();
  const chans_ty cs  = getChans();
  
  pgw << "<problemgraph name=\"" << m->name() << "_pg\" id=\"" << pgw.getId() << "\">" << std::endl;
  {
    pgw.indentUp();
    for ( typename nodes_ty::const_iterator iter = ns.begin();
          iter != ns.end();
          ++iter )
      (*iter)->assemble(pgw);
    for ( typename chans_ty::const_iterator c_iter = cs.begin();
          c_iter != cs.end();
          ++c_iter ) {
      hscd_port_list out = (*c_iter)->getOutputPorts();
      hscd_port_list in  = (*c_iter)->getInputPorts();
      
      for ( hscd_port_list::const_iterator ps_iter = out.begin();
            ps_iter != out.end();
            ++ps_iter ) {
        for ( hscd_port_list::const_iterator pd_iter = in.begin();
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
void hscd_graph_petri<T_node_type, T_chan_kind, T_chan_init_default>::
assemble( hscd_modes::PGWriter &pgw ) const {
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
        
        const hscd_root_port *port = dynamic_cast<const hscd_root_port *>(*iter);
        
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

template void hscd_graph_petri<hscd_choice_node, hscd_fifo_kind, hscd_fifo>::assemble
  (hscd_modes::PGWriter&) const;
template void hscd_graph_petri<hscd_transact_node, hscd_fifo_kind, hscd_fifo>::assemble
  (hscd_modes::PGWriter&) const;
template void hscd_graph_petri<hscd_fixed_transact_node, hscd_fifo_kind, hscd_fifo>::assemble
  (hscd_modes::PGWriter&) const;
template void hscd_graph_petri<hscd_choice_node, hscd_rendezvous_kind, hscd_rendezvous>::assemble
  (hscd_modes::PGWriter&) const;
