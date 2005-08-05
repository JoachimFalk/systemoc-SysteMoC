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


// InifiniBand Includes
#include "tt_ib.h"
#include "ib_m_atu.h"

using Expr::field;




class m_source: public smoc_actor {
  public:
    smoc_port_out<ct_queue2atu>   out1;
    smoc_port_out<ct_queue2atu>   out2;
    smoc_port_out<ct_bthgen2atu>  out3;
  private:
    int count;
        
    void process1() {
      std::cout << name() << " generating RQ token: " << std::endl;
      out1[0] = tt_notification(tt_notification::CLEAR_BUFFER, 0);
      std::cout << out1[0] << std::endl;
    }
    
    void process2() {
      std::cout << name() << " generating TQ token: " << std::endl;
      out2[0] = tt_notification(tt_notification::CLEAR_BUFFER, 0);
      std::cout << out2[0] << std::endl;
    }
    
    void process3() {
      std::cout << name() << " generating BTH/GRHGen token: " << std::endl;
      out3[0] = tt_raw_header("foo-bar", false, 0);
      std::cout << out3[0] << std::endl;
    }
    
    smoc_firing_state start;
  
  public:
    m_source( sc_module_name name ) :
      smoc_actor( name, start ),
      count(0)
    {
      start = out1(1) >> call(&m_source::process1) >> start
            | out2(1) >> call(&m_source::process2) >> start
            | out3(1) >> call(&m_source::process3) >> start;
    }
};

class m_sink: public smoc_actor {
  public:
    smoc_port_in<ct_bthgen2atu>   in1;
    smoc_port_in<ct_queue2atu>    in2;
  private:
   
    void print1() {
      std::cout << name() << ": got MFETCH token: ";
      std::cout << in1[0] << std::endl;
    }
    
    void print2() {
      std::cout << name() << ": got MSTORE token: ";
      std::cout << in2[0] << std::endl;
    }
    
    smoc_firing_state start;
  
  public:
    m_sink( sc_module_name name ) :
      smoc_actor( name, start )
    {
      start = in1(1) >> call(&m_sink::print1) >> start
            | in2(1) >> call(&m_sink::print2) >> start;
    }
};


class m_top
: public smoc_ndf_constraintset {
  public:
    m_top( sc_module_name name )
      : smoc_ndf_constraintset(name)
    {
      m_source      &src = registerNode(new m_source("src"));
      
      m_sink        &snk = registerNode(new m_sink("snk"));
      
      ib_m_atu      &atu = registerNode(new ib_m_atu("atu",std::cout));

      assert( dynamic_cast<smoc_root_node *>(&atu) != NULL );
      
      connectNodePorts( src.out1, atu.in_rq2atu );
      connectNodePorts( src.out2, atu.in_tq2atu );
      connectNodePorts( src.out3, atu.in_bth_grh_gen2atu );
      
      connectNodePorts( atu.out_atu2mfetch, snk.in1 );
      connectNodePorts( atu.out_atu2mstore, snk.in2 );
    }
};

int sc_main (int argc, char **argv) {
  smoc_top_moc<smoc_ndf_moc<m_top> > top("top");
  
  sc_start(-1);
  return 0;
}
