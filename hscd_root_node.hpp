// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_ROOT_NODE_HPP
#define _INCLUDED_HSCD_ROOT_NODE_HPP

#include <hscd_root_port_list.hpp>
#include <hscd_port.hpp>
#include <hscd_op.hpp>
#include <hscd_director.hpp>
#ifndef __SCFE__
# include <hscd_pggen.hpp>
#endif

//#include <oneof.hpp>

#include <systemc.h>

#include <list>

/*
#define linkme(instance,port,chan) do {     \
  (instance).registerPort((instance).port); \
  (instance).port(chan);                    \
} while ( 0 )
*/

#define linkme(instance,port,chan) do {     \
  (instance).port(chan);                    \
} while ( 0 )

class hscd_opbase_node
  : public hscd_root_node_op_if {
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
    
    hscd_opbase_node()
      : hscd_root_node_op_if() {}
    
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
  :
#ifndef __SCFE__
    public hscd_modes::hscd_modes_base_structure,
#endif
    public hscd_opbase_node {
  protected:
    void startTransact( hscd_op_transact op ) { startOp(op); }
    void startChoice( hscd_op_choice op ) { startOp(op); }
    
    void transact( hscd_op_transact op ) {
      startTransact(op); waitFinished();
      startTransact(fire_port(1)); waitFinished();
      Director::getInstance().getResource( my_module()->name() ).compute( my_module()->name() );
    }
    
    void choice( hscd_op_choice op ) {
      startChoice(op); waitFinished();
      startTransact(fire_port(1)); waitFinished();
      Director::getInstance().getResource( my_module()->name() ).compute( my_module()->name() );
    }
    
    hscd_root_node()
      : hscd_opbase_node() {}
  public:
    //sc_event		_fire;
    hscd_port_in<void>  fire_port;
    
    virtual
    const sc_module *my_module() const = 0;
#ifndef __SCFE__
    virtual
    void assemble( hscd_modes::PGWriter &pgw ) const {
      return leafAssemble(my_module(),pgw); }
    
    void leafAssemble( const sc_module *m, hscd_modes::PGWriter &pgw ) const;
#endif
};

#endif // _INCLUDED_HSCD_ROOT_NODE_HPP
