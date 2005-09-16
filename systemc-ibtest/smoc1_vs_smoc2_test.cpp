// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>

#include <hscd_node_types.hpp>
#include <hscd_port.hpp>

#include <hscd_fifo.hpp>
#include <hscd_structure.hpp>
#include <hscd_scheduler.hpp>

#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_fifo.hpp>
#include <smoc_node_types.hpp>
#include <smoc_pggen.hpp>

#ifndef __SCFE__
#endif


using Expr::field;

class actor_v2: public smoc_actor {
public:
   smoc_port_out<int> output;
  //smoc_port_in<int> input;
private:
  int count;
  bool state;
  bool guard1() const{
    return state;
  }
  bool guard2() const{
    return !guard1();
  }
  void setGuard(){
    state=!state;
  }
    void process_produce() {
      ++count;
      std::cerr << "actor_v2::process_produce(), generating: "<< count << std::endl;
      output[0]=count;
    }
    /*void process_consume() {
    std::cerr << "actor_v2::process_consume(), eating     : "<< input[0] << std::endl;
    } */ 
  smoc_firing_state start;
  
public:
  actor_v2( sc_module_name name ) :
    smoc_actor( name, start ), 
    state(false),
    count(0)
  {
    start = !guard(&actor_v2::guard1)>> output(1) >> CALL(actor_v2::process_produce) >> start;
    
    // start = guard(&actor_v2::guard2) >> CALL(actor_v2::setGuard) >> start;

    //start = (input(1) && guard(&actor_v2::guard1)) >> CALL(actor_v2::process_consume) >> start;


    

  }
};







/*******************************************************************************
 ******************************************************************************/

class node_v1 :
  public hscd_transact_active_node
{
public:
  hscd_port_in<int> input;
  //hscd_port_out<int> output;

private:
  int count;
 
  template<class T, class U> void put_msg(T &port, const U &msg)
  {
    port[0] = msg;
    transact(port(1));

    wait(SC_ZERO_TIME);
  }

  void process(){
    while ( 1 ) {
      /*  ++count;
      std::cerr << "node_v1::process(), generating : "<< count << std::endl;
      output[0]=count;
      transact(output(1));
      */
      transact( input(2) );
      std::cerr << "node_v1::process(), eating     : "<< input[0]<< ", "<< input[1] << std::endl;
    }
  }

public:

  node_v1(sc_module_name name) :
    hscd_transact_active_node(name)  {}
};


/*******************************************************************************
 ******************************************************************************/


class m_top : public hscd_fifocsp_structure  {
public:
  m_top( sc_module_name name )
    : hscd_fifocsp_structure(name)
  {
    actor_v2 &ac2 = registerNode(new actor_v2("ac2")); 
    node_v1  &no1 = registerNode(new node_v1("no1"));

    connectNodePorts<2>(ac2.output,  no1.input);
    //    connectNodePorts(no1.output,  ac2.input); 
  }
};

int sc_main (int argc, char **argv) {
  m_top top("top");
  
  sc_start(-1);
  return 0;
}
