#include <cstdlib>
#include <iostream>
#include <smoc_structure.hpp>
#include <smoc_scheduler.hpp>
#include <smoc_port.hpp>
#include <smoc_fifo.hpp>
#include <smoc_node_types.hpp>
#include <smoc_pggen.hpp>

// SOURCE
class m_source : public smoc_fixed_transact_active_node {
    public:
      smoc_port_out<int> out;
    private:
      int increment;

    private:
      void process() {
          int i = 0;

          while(true) {
              i += increment;
              std::cout << "Generating value: " << i << std::endl;
              out[0] = i;
              transact();
          }
      }

    public:
      m_source( sc_module_name name, int step)
          : smoc_fixed_transact_active_node(name, out(1)) {
              increment = step;
          }

}; // end class m_source


// SINK
class m_sink : public smoc_fixed_transact_active_node {
    public:
        smoc_port_in<int> in;
    private:
        void process() {
            while(true) {
                std::cout << "Value Received: " << in[0] << std::endl;
                transact();
            }
        }
                 
    public:
        m_sink(sc_module_name name)
            : smoc_fixed_transact_active_node(name, in(1)) {}
        
}; // end class m_sink

// ADDER
class m_adder : public smoc_fixed_transact_active_node {
    public:
        smoc_port_in<int> in1, in2;
        smoc_port_out<int> out;
    
    private:
        void process() {
            while(true) {
                out[0] = in1[0] + in2[0];
                transact();
            }
        }

    public:
        m_adder(sc_module_name name) 
            : smoc_fixed_transact_active_node(name, in1(1) & in2(1) & out(1)) {}
}; // end class m_adder

class adder_nw : public smoc_sdf_structure {
    private:
        smoc_scheduler_asap *asap;
    public:
        adder_nw(sc_module_name mname) : smoc_sdf_structure( mname ) {
        
            // instantiate nodes
            m_source &source1 = registerNode(new m_source("SOURCE1",2));
            m_source &source2 = registerNode(new m_source("SOURCE2",3));
            m_sink &sink = registerNode(new m_sink("SINK"));
            m_adder &adder = registerNode(new m_adder("ADDER"));
        
            // connect nodes
            connectNodePorts(source1.out, adder.in1);
            connectNodePorts(source2.out, adder.in2);
            connectNodePorts(adder.out, sink.in);
            asap = new smoc_scheduler_asap("asap", getNodes());
        }
}; // end class adder_nw


// SC_MAIN
int sc_main (int argc, char **argv) {
  adder_nw anw("ADDERNW");
  sc_start(-1);
  return 0;
}; // end sc_main

