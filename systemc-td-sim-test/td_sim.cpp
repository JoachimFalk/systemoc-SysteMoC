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

class td_source: public smoc_actor {
public:
  smoc_port_out<int> out_ramp;
private:
  int count;
  void process() {
    if(count>8)sc_stop();
    std::cerr << name() << " generating: ";
    out_ramp[0] = 42;
    count++;
    std::cerr << out_ramp[0] << std::endl;
  }
  
  smoc_firing_state start;
  
public:
  td_source( sc_module_name name ) :
    smoc_actor( name, start ),
    count(0)
  {
    start = out_ramp(1) >> call(&td_source::process) >> start;
  }
};


class td_ramp: public smoc_actor {
public:
  smoc_port_in<int> in;
private:
   
  void print() {
    std::cerr << name() << ": got token: ";
    std::cerr << in[0] << std::endl;
  }
    
  smoc_firing_state start;
  
public:
  td_ramp( sc_module_name name ) :
    smoc_actor( name, start )
  {
    start = in(1) >> CALL(td_ramp::print) >> start;
  }
};


class m_top : public smoc_graph {
public:
  m_top( sc_module_name name )
    : smoc_graph(name)
  {
    td_source      &src  = registerNode(new td_source("src"));
      
    td_ramp        &ramp = registerNode(new td_ramp("rmp"));
    connectNodePorts( src.out_ramp, ramp.in );
  }
};

int sc_main (int argc, char **argv) {
  smoc_top_moc<m_top> top("top");
  
  sc_start(-1);
  return 0;
}
