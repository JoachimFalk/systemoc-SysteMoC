// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>

#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_fifo.hpp>
#include <smoc_node_types.hpp>

#include <callib.hpp>

#include "Video_XML/IDCTaddsub-out.xml"
/*
#include "Video_XML/IDCTclip-out-out.xml"
#include "Video_XML/IDCTclip-out.xml"
#include "Video_XML/IDCTfly-out-out.xml"
#include "Video_XML/IDCTfly-out.xml"
#include "Video_XML/IDCTscale-out-out.xml"
#include "Video_XML/IDCTscale-out.xml"
#include "Video_XML/MCadd-out.xml"
#include "Video_XML/Upsample-out.xml"
#include "Video_XML/block2row-out.xml"
#include "Video_XML/byte2bit-out.xml"
#include "Video_XML/col2block-out.xml"
#include "Video_XML/dequant-out.xml"
#include "Video_XML/parser-out.xml"
#include "Video_XML/reconstruct-out.xml"
#include "Video_XML/sequence-out.xml"
#include "Video_XML/transpose-out.xml"
*/

/*
 * graph example
 *

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
    i += step;
  }
  
  smoc_firing_state start;
public:
  m_source( sc_module_name name,int init_value=0, int step=1)
    :smoc_actor( name, start ), foo(1), i(init_value) , step(step) {
    start = (out.getAvailableSpace() >= var(foo)) >>
            call(&m_source::process)              >> start;
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
    : smoc_actor( name, start ) {
    start = in(1) >> call(&m_sink::process) >> start;
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

    connectNodePorts( src.out, blidct.I, smoc_fifo<int>(64));
    connectNodePorts( src1.out, blidct.MIN, smoc_fifo<int>(2));
    connectNodePorts( blidct.O, snk.in, smoc_fifo<int>(2));
  }
};

*/
int sc_main (int argc, char **argv) {
//  smoc_top_moc<IDCT2d_TEST> top("top");
  
  sc_start(-1);
  return 0;
}

