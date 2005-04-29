// vim: set sw=2 ts=8:

#include <hscd_vpc_Director.h>
#include <hscd_root_node.hpp>
// #include <systemc/kernel/sc_object_manager.h>

void hscd_root_node::leafAssemble( const sc_module *m, hscd_modes::PGWriter &pgw ) const {
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
      if ( *iter == &fire_port )
        continue;
      
      const hscd_root_port *port = dynamic_cast<const hscd_root_port *>(*iter);
      
      if ( !port )
        continue;
      pgw << "<port name=\"" << (*iter)->name() << "\" "
          << "type=\"" << (port->isInput ? "in" : "out") << "\" "
          << "id=\"" << pgw.getId(port) << "\"/>" << std::endl;
    }
//      for ( ports_ty::const_iterator iter = ports.begin();
//            iter != ports.end();
//            ++iter )
//        pgw << (*iter) /*->name()*/ << std::endl;
    pgw.indentDown();
  }
  pgw << "</process>" << std::endl;
}

void hscd_root_node::transact( hscd_op_transact op ) {
  startTransact(op); waitFinished();
  startTransact(fire_port(1)); waitFinished();
  Director::getInstance().getResource( my_module()->name() ).compute( my_module()->name() );
}

void  hscd_root_node::choice( hscd_op_choice op ){
  startChoice(op); waitFinished();
  startTransact(fire_port(1)); waitFinished();
  Director::getInstance().getResource( my_module()->name() ).compute( my_module()->name() );
}
