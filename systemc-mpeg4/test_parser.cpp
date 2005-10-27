
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdlib.h>

#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_fifo.hpp>
#include <smoc_node_types.hpp>
#ifndef __SCFE__
//# include <smoc_scheduler.hpp>
# include <smoc_pggen.hpp>
#endif

#include <callib.hpp>

#include "block_parser.hpp"

#define INAMEblk "test_in.bit"
#define ONAMEblk "test_out.dat"

class m_source_parser: public smoc_actor {
  public:
    smoc_port_out<int> out;
  
  private:
    bool isStreamGood;
    char ch;
    
    std::ifstream i1; 
    
    void process() {
      assert( !i1.eof() );
      out[0] = ch; ch = i1.get();
      
      std::cout << name() << "  write " << out[0] << std::endl;
      
      isStreamGood = !i1.eof() && i1.good();
    }
    
    smoc_firing_state start;
  public:
    m_source_parser( sc_module_name name ) 
      :smoc_actor( name, start ) {
      i1.open(INAMEblk, ios::binary);
      
      ch = i1.get();
      
      isStreamGood = !i1.eof() && i1.good();
      
      start = (out.getAvailableSpace() >= 1 &&
               var(isStreamGood))
              >> CALL(m_source_parser::process)
              >> start;
    }
  ~m_source_parser( ){
        i1.close();
  }
};

class m_sink: public smoc_actor {
  public:
    smoc_port_in<int> in;
  
  private:
    //std::ofstream fo; 
    
    void process() {
      std::cout << name() << " receiving " << in[0] << std::endl;
     // fo << in[0] << std::endl;
    }
    
    smoc_firing_state start;
  public:
    m_sink( sc_module_name name )
      : smoc_actor( name, start )/*,
        fo(ONAMEblk)*/ {
      start = in(8) >> CALL(m_sink::process)  >> start;
    }
    
    ~m_sink() {
      //fo.close();
    
    }
};

class m_list_sink: public smoc_actor {
  public:
    smoc_port_in<cal_list<int>::t> in;
  private:
    void process() {
      std::cout << name() << " receiving " << in[0] << std::endl;
    }
    
    smoc_firing_state start;
  public:
    m_list_sink( sc_module_name name )
      :smoc_actor( name, start ) {
      start = in(64) >> CALL(m_list_sink::process)  >> start;
    }
};

class PARSER_TEST
: public smoc_graph {
private:
  m_source_parser src_parser;
  m_block_parser  blparser;
  m_sink        snk0;
  m_list_sink   snk1;
  m_sink        snk2;
  m_sink        snk3;
public:
  PARSER_TEST( sc_module_name name )
    : smoc_graph(name),
      src_parser("src_parser"),
      blparser("blparser"),
      snk0("snk0"), 
      snk1("snk1"), 
      snk2("snk2"),
      snk3("snk3"){
    connectNodePorts( src_parser.out, blparser.I, smoc_fifo<int>(256));
    connectNodePorts( blparser.O0, snk0.in, smoc_fifo<int>(256));
    connectNodePorts( blparser.O1, snk1.in, smoc_fifo<cal_list<int>::t >(256));
    connectNodePorts( blparser.O2, snk2.in, smoc_fifo<int>(256));
    connectNodePorts( blparser.O3, snk3.in, smoc_fifo<int>(256));
      }
};

int sc_main (int argc, char **argv) {
  smoc_top_moc<PARSER_TEST> top("top");
  
#define GENERATE "--generate-problemgraph"
  if (argc > 1 && 0 == strncmp(argv[1], GENERATE, sizeof(GENERATE))) {
    smoc_modes::dump(std::cout, top);
  } else {  
    sc_start(-1);
  }
#undef GENERATE
  return 0;
}
