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

#include <callib.hpp>

#include "block_idct.hpp"

class m_source: public smoc_actor {
  public:
    smoc_port_out<int> out;
  private:
    size_t foo;
    int i;
    const int step;

    //std::ifstream i1;

    void process() {
      //int data;
      //std::cout << name() << " generating " << i << std::endl;
     // if(i1.good()){
       // i1 >> data;
        out[0] = i;//data;
        cout << name() << "  write " << out[0]/*data*/ << std::endl;
        i+=step;
      //}else{
       // cout << "  file empty" << std::endl;
      //}
    }
    
    smoc_firing_state start;
  public:
    m_source( sc_module_name name,int init_value=0, int step=1)
      :smoc_actor( name, start ), i(init_value) , step(step){
      foo = 1;
      //i1.open("test.txt");
      start = ((out.getAvailableSpace() >= 1) && (var(i) <= 65536)) >>
              CALL(m_source::process)               >> start;
    }
};

class m_sink: public smoc_actor {
  public:
    smoc_port_in<int> in;
  private:

    //std::ostream &o;

    void process() {
      std::cout << name() << " receiving " << in[0] << std::endl;
      //o << in[0];
    }
    
    smoc_firing_state start;
  public:
    m_sink( sc_module_name name )
      :smoc_actor( name, start ){
      start = in(1) >> CALL(m_sink::process)  >> start;
    }
};

class IDCT2d_TEST
: public smoc_graph {
  public:
    IDCT2d_TEST( sc_module_name name )
      : smoc_graph(name) {
      
      
      m_source      &src    = registerNode(new m_source("src"));
      m_source      &src1   = registerNode(new m_source("src1"));
      m_block_idct  &blidct = registerNode(new m_block_idct("blidct"));
      m_sink        &snk    = registerNode(new m_sink("snk"));

      connectNodePorts( src.out, blidct.I, smoc_fifo<int>(128));
      connectNodePorts( src1.out, blidct.MIN, smoc_fifo<int>(2));
      connectNodePorts( blidct.O, snk.in, smoc_fifo<int>(128));
    }
};

int sc_main (int argc, char **argv) {
  smoc_top_moc<IDCT2d_TEST> top("top");
  
#define GENERATE "--generate-problemgraph"
  if (argc > 1 && 0 == strncmp(argv[1], GENERATE, sizeof(GENERATE))) {
    smoc_modes::dump(std::cout, top);
  } else {  
    sc_start(-1);
  }
#undef GENERATE
  return 0;
}
