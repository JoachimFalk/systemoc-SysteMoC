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

class m_source_idct: public smoc_actor {
  public:
    smoc_port_out<int> out;
    smoc_port_out<int> min;
  private:
    int i;
    
    //std::ifstream i1;
    
    void process() {
      //int data;
      //std::cout << name() << " generating " << i << std::endl;
      // if(i1.good()){
       // i1 >> data;
        min[0] = -256;
        for ( int j = 0; j <= 63; ++j ) {
          out[j] = i++;
          cout << name() << "  write " << out[j] << std::endl;
        }
      //}else{
       // cout << "  file empty" << std::endl;
      //}
    }
    
    smoc_firing_state start;
  public:
    m_source_idct( sc_module_name name,int init_value = 0 )
      :smoc_actor( name, start ), i(init_value) {
      //i1.open("test.txt");
      start = ((out.getAvailableSpace() >= 64) &&
               (min.getAvailableSpace() >= 1) &&
               (var(i) <= 655))
              >> CALL(m_source_idct::process)
              >> start;
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

class m_source: public smoc_actor {
  public:
    smoc_port_out<int> out;
  private:
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
      //i1.open("test.txt");
      start = ((out.getAvailableSpace() >= 1) && (var(i) <= 655)) >>
              CALL(m_source::process)               >> start;
    }
};

class IDCT2d_TEST
: public smoc_graph {
  public:
    IDCT2d_TEST( sc_module_name name )
      : smoc_graph(name) {
      
      
      m_source_idct &src_idct = registerNode(new m_source_idct("src_idct"));
      m_block_idct  &blidct   = registerNode(new m_block_idct("blidct"));
      m_sink        &snk      = registerNode(new m_sink("snk"));
      
      connectNodePorts( src_idct.out, blidct.I,   smoc_fifo<int>(128));
      connectNodePorts( src_idct.min, blidct.MIN, smoc_fifo<int>(2));
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
