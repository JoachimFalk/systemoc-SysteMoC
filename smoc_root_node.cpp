// vim: set sw=2 ts=8:

#include <smoc_root_port.hpp>
#include <smoc_root_node.hpp>
// #include <systemc/kernel/sc_object_manager.h>

smoc_root_node::smoc_root_node(const smoc_firing_state &s)
  : _currentState(s), _initialState(NULL), is_v1_actor(false)
#ifdef ENABLE_SYSTEMC_VPC
  , commstate( smoc_activation_pattern(Expr::till(vpc_event), true) >>
	       smoc_interface_action(smoc_func_diverge(
						       this,&smoc_root_node::communicate)) ) 
#endif // ENABLE_SYSTEMC_VPC
  {}
smoc_root_node::smoc_root_node(smoc_firing_state &s)
  : _initialState(&s), is_v1_actor(false)
#ifdef ENABLE_SYSTEMC_VPC
  , commstate( smoc_activation_pattern(Expr::till(vpc_event), true) >>
	       smoc_interface_action(smoc_func_diverge(
						       this,&smoc_root_node::communicate)) )
#endif // ENABLE_SYSTEMC_VPC
  {}

#ifdef ENABLE_SYSTEMC_VPC
const smoc_firing_state &smoc_root_node::communicate() {
    
# ifdef SYSTEMOC_DEBUG
  std::cout << "  <call actor=" << myModule()->name()
	    << " func=smoc_root_node::communicate>" << std::endl;
  std::cout << "    <communication type=\"execute\"/>" << std::endl;
# endif
    
  assert( vpc_event );
    
  for ( smoc_port_list::iterator iter = ports_setup.begin();
	iter != ports_setup.end();
	++iter ) {
    (*iter)->commExec();
    (*iter)->reset();
  }
  ports_setup.clear();
# ifdef SYSTEMOC_DEBUG
  std::cout << "  </call>"<< std::endl;
# endif
  return nextState;
}
#endif // ENABLE_SYSTEMC_VPC

void smoc_root_node::finalise() {
  //    std::cout << myModule()->name() << ": finalise" << std::endl;
  if ( _initialState != NULL ) {
    _currentState = *_initialState;
    _initialState = NULL;
  }
  _currentState.finalise(this);
  //    dumpActor(std::cout);
}

const smoc_port_list smoc_root_node::getPorts() const {
  smoc_port_list   ports;
  const sc_module *m = myModule();
  
  // std::cout << "=== getPorts ===" << this << std::endl;
  for ( sc_pvector<sc_object*>::const_iterator iter = m->get_child_objects().begin();
        iter != m->get_child_objects().end();
        ++iter ) {
    smoc_root_port *port = dynamic_cast<smoc_root_port *>(*iter);
    
    if ( port != NULL )
      ports.push_back(port);
  }
  return ports;
}

void smoc_root_node::pgAssemble( smoc_modes::PGWriter &pgw, const smoc_root_node *n ) const
  {}

void smoc_root_node::assemble( smoc_modes::PGWriter &pgw ) const {
  const sc_module     *m  = myModule();
  const smoc_port_list ps = getPorts();
  
  if ( !ps.empty() ) {
    pgw << "<process name=\"" << m->name() << "\" id=\"" << pgw.getId(this) << "\">" << std::endl;
    pgw.indentUp();
    for ( smoc_port_list::const_iterator iter = ps.begin();
          iter != ps.end();
          ++iter )
      pgw << "<port name=\"" << (*iter)->name() << "\" "
          << "type=\"" << ((*iter)->isInput() ? "in" : "out") << "\" "
          << "id=\"" << pgw.getId(*iter) << "\"/>" << std::endl;
  }
  pgAssemble( pgw, this );
  if ( !ps.empty() ) {
    pgw.indentDown();
    pgw << "</process>" << std::endl;
  }
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
