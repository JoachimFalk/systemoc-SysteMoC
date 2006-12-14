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

#include "IDCT2d.hpp"
#include "IDCTaddsub.hpp"
#include "IDCTclip.hpp"
#include "IDCTfly.hpp"
#include "IDCTscale.hpp"
#include "dequant.hpp"
#include "Upsample.hpp"
#include "block2row.hpp"
#include "byte2bit.hpp"
#include "col2block.hpp"
#include "reconstruct.hpp"
#include "MCadd.hpp"
//#include "sequence.hpp"
#include "transpose.hpp"

class m_source: public smoc_actor {
  public:
    smoc_port_out<int> out;
  private:
    size_t foo;
    int i;
    const int step;

    void process() {
      std::cout << name() << " generating " << i << std::endl;
      out[0] = i;
      i+=step;
    }
    
    smoc_firing_state start;
  public:
    m_source( sc_module_name name,int init_value=0, int step=1)
      :smoc_actor( name, start ), i(init_value) , step(step){
      foo = 1;
      start = (out.getAvailableSpace() >= var(foo)) >>
              CALL(m_source::process)               >> start;
    }
};

class m_list_source: public smoc_actor {
  public:
    smoc_port_out<cal_list<int>::t> out;
  private:
    int i;
    
    void process() {
      cal_list<int>::t l(Integers(i,i+63));
      out[0] = l;
      std::cout << name() << " generating List " << l << std::endl;
      i += 35;
    }
    
    smoc_firing_state start;
  public:
    m_list_source( sc_module_name name )
      :smoc_actor( name, start ), i(0) {
      start =  out(1) >> CALL(m_list_source::process)  >> start;
    }
};

class m_sink: public smoc_actor {
  public:
    smoc_port_in<int> in;
  private:
    void process() {
      std::cout << name() << " receiving " << in[0] << std::endl;
    }
    
    smoc_firing_state start;
  public:
    m_sink( sc_module_name name )
      :smoc_actor( name, start ) {
      start = in(1) >> CALL(m_sink::process)  >> start;
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
      start = in(1) >> CALL(m_list_sink::process)  >> start;
    }
};

class m_top
: public smoc_graph {
  public:
    m_top( sc_module_name name )
      : smoc_graph(name) {
      
      
      m_idct        &idct1 = registerNode(new m_idct("idct1"));

      m_source      &src1  = registerNode(new m_source("src1"));
      m_source      &src2  = registerNode(new m_source("src2"));
      m_source      &src3  = registerNode(new m_source("src3"));
      m_source      &src4  = registerNode(new m_source("src4"));
      m_source      &src5  = registerNode(new m_source("src5"));
      m_source      &src6  = registerNode(new m_source("src6"));
      m_source      &src7  = registerNode(new m_source("src7"));
      m_source      &src8  = registerNode(new m_source("src8"));

      
      m_sink        &snk1  = registerNode(new m_sink("snk1"));
      m_sink        &snk2  = registerNode(new m_sink("snk2"));
      m_sink        &snk3  = registerNode(new m_sink("snk3"));
      m_sink        &snk4  = registerNode(new m_sink("snk4"));
      m_sink        &snk5  = registerNode(new m_sink("snk5"));
      m_sink        &snk6  = registerNode(new m_sink("snk6"));
      m_sink        &snk7  = registerNode(new m_sink("snk7"));
      m_sink        &snk8  = registerNode(new m_sink("snk8"));

      connectNodePorts( src1.out, idct1.i0, smoc_fifo<int>(2));
      connectNodePorts( src2.out, idct1.i1, smoc_fifo<int>(2));
      connectNodePorts( src3.out, idct1.i2, smoc_fifo<int>(2));
      connectNodePorts( src4.out, idct1.i3, smoc_fifo<int>(2));
      connectNodePorts( src5.out, idct1.i4, smoc_fifo<int>(2));
      connectNodePorts( src6.out, idct1.i5, smoc_fifo<int>(2));
      connectNodePorts( src7.out, idct1.i6, smoc_fifo<int>(2));
      connectNodePorts( src8.out, idct1.i7, smoc_fifo<int>(2));

      connectNodePorts( idct1.o0, snk1.in, smoc_fifo<int>(2));
      connectNodePorts( idct1.o1, snk2.in, smoc_fifo<int>(2));
      connectNodePorts( idct1.o2, snk3.in, smoc_fifo<int>(2));
      connectNodePorts( idct1.o3, snk4.in, smoc_fifo<int>(2));
      connectNodePorts( idct1.o4, snk5.in, smoc_fifo<int>(2));
      connectNodePorts( idct1.o5, snk6.in, smoc_fifo<int>(2));
      connectNodePorts( idct1.o6, snk7.in, smoc_fifo<int>(2));
      connectNodePorts( idct1.o7, snk8.in, smoc_fifo<int>(2));

    }
};

int sc_main (int argc, char **argv) {
  smoc_top_moc<m_top> top("top");
  sc_start(-1);
  return 0;
}
