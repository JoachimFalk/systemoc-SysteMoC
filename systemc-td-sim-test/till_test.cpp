#include <cstdlib>
#include <iostream>

#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_fifo.hpp>
#include <smoc_node_types.hpp>
#include <smoc_event.hpp>

smoc_event timeout;

class till_actor: public smoc_actor{

private:
  smoc_firing_state main;
  
  /**
   *
   */
  void process(){
    cerr << "Got timeout at: " << sc_time_stamp() << endl;
    smoc_reset(timeout);
  }
public:

//  smoc_event timeout;

  till_actor( sc_module_name name ) : smoc_actor ( name , main ){
    main = Expr::till( timeout )
      >> CALL(till_actor::process)
      >> main
      ;
  }
};

class till_top: public smoc_graph {
  SC_HAS_PROCESS(till_top);
protected:
  till_actor act;
  void time_out_process() {
    int i=10;
    while(i-- != 0){
      wait(120, SC_NS);
      smoc_notify(timeout);
      std::cout << "timeout send" << std::endl;
    }
  }
public:
  till_top( sc_module_name name ) : smoc_graph( name ), act("till_act") {
    registerNode(&act);
    SC_THREAD(time_out_process);
  }
};

int sc_main (int argc, char **argv) {
  smoc_top_moc<till_top> top("top");
  
  sc_start(-1);
  return 0;
}
