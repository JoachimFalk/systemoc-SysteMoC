// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>

#include <hscd_structure.hpp>
#include <hscd_scheduler.hpp>
#include <hscd_port.hpp>
#include <hscd_fifo.hpp>
#include <hscd_node_types.hpp>
#include <hscd_pggen.hpp>

template <typename T>
class m_adder: public hscd_fixed_transact_active_node {
  public:
    hscd_port_in<T>  in;
    hscd_port_out<T> out;
  private:
    void process() {
      while (true) {
	out[0] = in[0] + in[1];
	std::cout << "Adding " << in[0] << " + " << in[1] << " = " << out[0] << std::endl;
	transact();
      }
    }
  public:
    m_adder( sc_module_name name )
      :hscd_fixed_transact_active_node( name, in(2) & out(1) ) {}
};

class m_source: public hscd_fixed_transact_active_node {
  public:
    hscd_port_out<int> out;
  private:
    void process() {
      int i = 0;
      
      while (true) {
	std::cout << "Generating " << i << std::endl;
	out[0] = i++;
	transact();
      }
    }
  public:
    m_source( sc_module_name name )
      :hscd_fixed_transact_active_node( name, out(1) ) {}
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

class m_top_asap_scheduler
  : public hscd_scheduler_base {
  private:
    hscd_rendezvous<void> chan_source, chan_adder, chan_sink;
    hscd_port_out<void>   source, adder, sink;
    
    void schedule() {
      // while ( 1 ) choice( source(1) | adder(1) | sink(1) );
      while ( 1 ) {
        transact ( source(2) );
        transact ( adder(1) );
        transact ( sink(1) );
      }
    }
  public:
    SC_HAS_PROCESS(m_top_asap_scheduler);
    
    m_top_asap_scheduler( hscd_port_in<void> &fire_source,
                          hscd_port_in<void> &fire_adder, 
                          hscd_port_in<void> &fire_sink )
      : hscd_scheduler_base("m_top_asap_scheduler") {
      
      fire_source(chan_source); source(chan_source);
      fire_adder (chan_adder);  adder (chan_adder);
      fire_sink  (chan_sink);   sink  (chan_sink);
      SC_THREAD(schedule);
    }
};

class m_top
: public hscd_sdf_structure {
  private:
    m_top_asap_scheduler *asap;
  public:
    m_top( sc_module_name _name ): hscd_sdf_structure(_name) {
      m_source      &src   = registerNode(new m_source("src"));
      m_adder<int>  &adder = registerNode(new m_adder<int>("adder"));
      m_sink        &sink  = registerNode(new m_sink("sink"));
      
      connectNodePorts( src.out, adder.in );
      connectNodePorts( adder.out, sink.in );
      
      asap = new m_top_asap_scheduler( src.fire_port, adder.fire_port, sink.fire_port );
      // asap = new hscd_scheduler_asap( "asap", getNodes() );
    }
};

int sc_main (int argc, char **argv) {
  m_top        top("top");

  hscd_modes::dump( std::cout, top );
  
  sc_start(-1);
  return 0;
}
