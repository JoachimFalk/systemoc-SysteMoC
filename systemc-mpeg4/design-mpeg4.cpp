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
#include "parser.hpp"
//#include "sequence.hpp"
#include "transpose.hpp"

class m_source: public smoc_actor {
  public:
    smoc_port_out<int> out;
  private:
    size_t foo;
    int i;
    
    void process() {
      std::cout << name() << " generating " << i << std::endl;
      out[0] = i++;
    }
    
    smoc_firing_state start;
  public:
    m_source( sc_module_name name )
      :smoc_actor( name, start ), i(0) {
      foo = 1;
      start = (out.getAvailableSpace() >= var(foo)) >>
              call(&m_source::process)              >> start;
    }
};

class m_list_source: public smoc_actor {
  public:
    smoc_port_out<cal_list<int>::t> out;
  private:
    int i;
    
    void process() {
      out[0] = Integers(i,i+63);
      std::cout << name() << " generating List " << out[0] << std::endl;
      i += 35;
    }
    
    smoc_firing_state start;
  public:
    m_list_source( sc_module_name name )
      :smoc_actor( name, start ), i(0) {
      start =  out(1) >> call(&m_list_source::process) >> start;
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
      start = in(1) >> call(&m_sink::process) >> start;
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
      start = in(1) >> call(&m_list_sink::process) >> start;
    }
};

class m_top
: public smoc_ndf_constraintset {
  public:
    m_top( sc_module_name name )
      : smoc_ndf_constraintset(name) {
      m_list_source &lsrc1 = registerNode(new m_list_source("lsrc1"));
      m_source      &src1  = registerNode(new m_source("src1"));
      m_list_sink   &lsnk1 = registerNode(new m_list_sink("lsnk1"));
      m_dequant     &deq1  = registerNode(new m_dequant("deq1"));
      m_sink        &snk1  = registerNode(new m_sink("snk1"));
      m_sink        &snk2  = registerNode(new m_sink("snk2"));
      
      connectNodePorts( lsrc1.out, deq1.IN );
      connectNodePorts( src1.out, deq1.FLAGS, smoc_fifo<int>(3) );
      connectNodePorts( deq1.OUT, lsnk1.in );
      connectNodePorts( deq1.DC, snk1.in );
      connectNodePorts( deq1.MIN, snk2.in );
    }
};

int sc_main (int argc, char **argv) {
  smoc_top_moc<smoc_ndf_moc<m_top> > top("top");
  
  sc_start(-1);
  return 0;
}
