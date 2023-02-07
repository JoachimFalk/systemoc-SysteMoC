// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_node_types.hpp>
#ifndef __SCFE__
//# include <smoc_scheduler.hpp>
# include <systemoc/smoc_pggen.hpp>
#endif

using namespace std;

class m_source: public smoc_actor {
  public:
    smoc_port_out<int> out;
  private:
    int i;
    
    void process() {
#ifndef NDEBUG
      cout << name() << " generating " << i << std::endl;
#endif
      out[0] = i++;
    }

    smoc_firing_state start;
  public:
    m_source( sc_module_name name )
      :smoc_actor( name, start ), i(0) {
      start =  out(1) >> (VAR(i) < 1000000) >> CALL(m_source::process) >> start;
    }
};

class m_sink: public smoc_actor {
  public:
    smoc_port_in<int> in;
  private:
    void process() {
#ifndef NDEBUG
      cout << name() << " receiving " << in[0] << std::endl;
#endif
    }
    
    smoc_firing_state start;
  public:
    m_sink( sc_module_name name )
      :smoc_actor( name, start ) {
      start = (in(1) && in.getValueAt(0) == 0) 
				>> CALL(m_sink::process) 
				>> start
				| in(1) 
				>> CALL(m_sink::process) 
				>> start;
    }
};

class m_top
: public smoc_graph {
  public:
    m_top( sc_module_name name )
      : smoc_graph(name) {
      m_source      &src1 = registerNode(new m_source("src1"));
      m_sink        &sink = registerNode(new m_sink("sink"));
#ifndef KASCPAR_PARSING      
      connectNodePorts( src1.out, sink.in, smoc_fifo<int>(2) );
#endif
    }
};

int sc_main (int argc, char **argv) {
  smoc_top_moc<m_top> top("top");
  
  sc_start();

  return 0;
}
