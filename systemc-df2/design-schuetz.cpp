// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>

#include <hscd_structure.hpp>
#include <hscd_scheduler.hpp>
#include <hscd_port.hpp>
#include <hscd_fifo.hpp>
#include <hscd_node_types.hpp>
#include <hscd_pggen.hpp>

class m_source: public hscd_fixed_transact_active_node {
  public:
    hscd_port_out<int> out;
  private:
    void process() {
      int i = 0;
      
      while (true) {
	std::cout << "Generating " << i << std::endl;
        wait(10,SC_NS);
	out[0] = i++;
	transact();
      }
    }
  public:
    m_source( sc_module_name name )
      :hscd_fixed_transact_active_node( name, out(1) ) {}
};

template <typename T>
class m_merge: public hscd_choice_active_node {
  public:
    hscd_port_in<T>  in1, in2;
    hscd_port_out<T> out;
  private:
    void process() {
      while (true) {
        choice( in1(1) | in2(1) );
        if ( in1 )
          out[0] = in1[0];
        else
          out[0] = in2[0];
        transact( out(1) );
      }
    }
  public:
    m_merge( sc_module_name name )
      :hscd_choice_active_node( name ) {}
};

class m_sink: public hscd_fixed_transact_active_node {
  public:
    hscd_port_in<int> in;
  private:
    void process() {
      while (true) {
	std::cout << "Received " << in[0] << std::endl;
	transact();
      }
    }
  public:
    m_sink( sc_module_name name )
      :hscd_fixed_transact_active_node( name, in(1) ) {}
};

class m_top
: public hscd_fifocsp_structure {
  private:
    hscd_scheduler_asap *asap;
  public:
    m_top( sc_module_name _name ): hscd_fifocsp_structure(_name) {
      m_source      &src1  = registerNode(new m_source("src1"));
      m_source      &src2  = registerNode(new m_source("src2"));
      m_merge<int>  &merge = registerNode(new m_merge<int>("merge"));
      m_sink        &sink  = registerNode(new m_sink("sink"));
      
      connectNodePorts( src1.out, merge.in1 );
      connectNodePorts( src2.out, merge.in2 );
      connectNodePorts( merge.out, sink.in );
      asap = new hscd_scheduler_asap( "asap", getNodes() );
    }
};

int sc_main (int argc, char **argv) {
  m_top        top("top");

//  hscd_modes::dump( std::cout, top );
  
  sc_start(-1);
  return 0;
}
