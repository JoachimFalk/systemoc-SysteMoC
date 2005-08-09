// vim: set sw=2 ts=8:

#include <smoc_root_node.hpp>
// #include <systemc/kernel/sc_object_manager.h>

void smoc_root_node::assemble( smoc_modes::PGWriter &pgw ) const {
  const sc_module *m = myModule();
  
  pgw << "<process name=\"" << m->name() << "\" id=\"" << pgw.getId(this) << "\">" << std::endl;
  {
//    sc_object_manager *om = simcontext()->get_object_manager();
//    
    pgw.indentUp();
#if 0
    pgw << om->hierarchy_curr() << std::endl;
    if ( om->hierarchy_curr() != NULL )
      pgw << om->hierarchy_curr()->name() << std::endl;
    for ( sc_object *foo = om->first_object();
          foo != NULL;
          foo = om->next_object() )
      pgw << foo->name() << std::endl;
    pgw << "-------------------------------" << std::endl;
#endif

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
    pgw.indentDown();
  }
  pgw << "</process>" << std::endl;
}

smoc_port_list &smoc_root_node::getPorts() {
  if ( !ports_valid ) {
    sc_module      *m = myModule();
    
//    std::cout << "=== getPorts ===" << this << std::endl;
    for ( sc_pvector<sc_object*>::const_iterator iter = m->get_child_objects().begin();
          iter != m->get_child_objects().end();
          ++iter ) {
      sc_port_base *port = dynamic_cast<sc_port_base *>(*iter);
      if ( port != NULL && port->kind() == smoc_root_port::kind_string )
        ports.push_back(reinterpret_cast<smoc_root_port *>(port));
    }
    ports_valid = true;
  }
  return ports;
}

std::ostream &smoc_root_node::dumpActor(std::ostream &o) {
  o << "actor: " << myModule()->name() << std::endl;
  smoc_port_list ps = getPorts();
  o << "  ports:" << std::endl;
  for ( smoc_port_list::const_iterator iter = ps.begin();
        iter != ps.end();
        ++iter ) {
    o << "  " << *iter << std::endl;
  }
  return o;
}
