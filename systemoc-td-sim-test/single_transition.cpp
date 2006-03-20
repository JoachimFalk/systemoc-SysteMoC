// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>

#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_fifo.hpp>
#include <smoc_node_types.hpp>
#ifndef __SCFE__
//# include <smoc_scheduler.hpp>
# include <smoc_pggen.hpp>
#endif


using Expr::field;

class single_transition: public smoc_actor {
public:
  smoc_port_out<int> out;
  smoc_port_in<int> in;
private:
  int count;
  void process() {
    std::cerr << "singele_transition::process()" << std::endl;
    TraceLog << "singele_transition::process()" << std::endl;
  }
  
  smoc_firing_state start;
  
public:
  single_transition( sc_module_name name ) :
    smoc_actor( name, start ), 
    count(0)
  {
    start = out(1) >> CALL(single_transition::process) >> start;
  }
};





class m_top : public smoc_graph {
public:
  m_top( sc_module_name name )
    : smoc_graph(name)
  {
    single_transition    &src  = registerNode(new single_transition("single"));
    connectNodePorts(src.out,src.in);
  }
};

int sc_main (int argc, char **argv) {
  smoc_top_moc<m_top> top("top");
  
  dump(cerr,top);

  sc_start(-1);
  return 0;
}
