// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_ROOT_NODE_HPP
#define _INCLUDED_HSCD_ROOT_NODE_HPP

#include <hscd_root_port_list.hpp>
#include <hscd_port.hpp>
#include <hscd_op.hpp>
#include <hscd_pggen.hpp>

//#include <oneof.hpp>

#include <systemc.h>

#include <list>

#define linkme(instance,port,chan) do {     \
  (instance).registerPort((instance).port); \
  (instance).port(chan);                    \
} while ( 0 )

class hscd_opbase_node
  : public sc_module,
    public hscd_root_node_op_if,
    public hscd_modes::hscd_modes_base_structure {
  private:
//    typedef std::list<hscd_root_port *> ports_ty;
//    ports_ty                            ports;
  protected:
    void startTransact( hscd_op_transact op ) { startOp(op); }
    void startChoice( hscd_op_choice op ) { startOp(op); }
    
    void transact( hscd_op_transact op ) {
      startTransact(op); waitFinished();
    }
    
    void choice( hscd_op_choice op ) {
      startChoice(op); waitFinished();
    }
    
    hscd_opbase_node( sc_module_name name )
      :sc_module(name) {}
    
    sc_event		  _opFinished;
    
    void waitFinished() {
      if ( !finished() )
	wait( _opFinished );
      assert( finished() );
    }
    
    void opFinished() { _opFinished.notify_delayed(); }
  public:
//    void registerPort( hscd_root_port &port ) {
//      ports.push_back(&port);
//    }
};

class hscd_root_node
  : public hscd_opbase_node {
  protected:
    void startTransact( hscd_op_transact op ) { startOp(op); }
    void startChoice( hscd_op_choice op ) { startOp(op); }
    
    void transact( hscd_op_transact op ) {
      startTransact(op); waitFinished();
      startTransact(fire_port(1)); waitFinished();
    }
    
    void choice( hscd_op_choice op ) {
      startChoice(op); waitFinished();
      startTransact(fire_port(1)); waitFinished();
    }
    
    hscd_root_node( sc_module_name name )
      :hscd_opbase_node(name) {}
  public:
    //sc_event		_fire;
    hscd_port_in<void>  fire_port;

    void assemble( hscd_modes::PGWriter &pgw ) const;
};

#endif // _INCLUDED_HSCD_ROOT_NODE_HPP
