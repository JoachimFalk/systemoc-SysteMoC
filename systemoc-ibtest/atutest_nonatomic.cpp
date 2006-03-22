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
#include "ib_m_atu_nonatomic.h"

// number of messages that will be completely processed (src -> snk)
// the m_source module might produce more messages, sim stops after
// NUM_TEST_MSGS were received at m_sink module
#define NUM_TEST_MSGS 50

using Expr::field;
using std::endl;
using std::cout;



class m_source: public smoc_actor {
  public:
    smoc_port_out<ct_queue2atu>   out1;
    smoc_port_out<ct_queue2atu>   out2;
    smoc_port_out<ct_bthgen2atu>  out3;
  private:
    int count;
    
    // generate a random RQ message for testing
    void process1() {
      
      // decide which of 3 oneof types to use for next message
      switch ( count % 3 ) { 
      
        case 0 :
          // generate a dummy notification
          cout << name() << " alias RQ: generating message( "<< count << ") :" << endl;
          out1[0] = tt_notification(tt_notification::CLEAR_BUFFER, 0);
          cout << out1[0] << endl;
          break;

        case 1 :
          // generate a dummy buffer offset notification
          cout << name() << " alias RQ: generating message( "<< count << ") :" << endl;
          out1[0] = tt_bo_notification(12, 10);
          cout << out1[0] << endl;
          break;

        case 2:
          // generate a dummy data transmit token
          cout << name() << " alias RQ: generating message( "<< count << ") :" << endl;
          
          tt_data tt(16, true);
          ts_scaga_buf x = { 65536, 64, 124};
          tt.buffer = x;
          
          out1[0] = tt;
          cout << out1[0] << endl;
          break;
      }
      count++;
    }
    
    // generate a random TQ message for testing
    void process2() {
     
      // decide which of 3 oneof types to use for next message
      switch ( count % 3 ) { 
      
        case 0 :
          // generate a dummy notification
          cout << name() << " alias TQ: generating message( "<< count << ") :" << endl;
          out2[0] = tt_notification(tt_notification::CLEAR_BUFFER, 2);
          cout << out2[0] << endl;
          break;

        case 1 :
          // generate a dummy buffer offset notification
          cout << name() << " alias TQ: generating message( "<< count << ") :" << endl;
          out2[0] = tt_bo_notification(128, 17);
          cout << out2[0] << endl;
          break;

        case 2:
          // generate a dummy data transmit token
          cout << name() << " alias TQ: generating message( "<< count << ") :" << endl;
          
          tt_data tt(5, true);
          ts_scaga_buf x = { 0, 256, 17};
          tt.buffer = x;
          
          out2[0] = tt;
          cout << out2[0] << endl;
          break;
      }
      count++;
 
    }
    
    // generate a random BTH/GRHGen message for testing
    void process3() {
     
      // decide which of 2 oneof types to use for next message
      switch ( count % 2 ) { 
      
        case 0 :
          // generate a dummy raw header transmit token
          cout << name() << " alias BTH/GRHGen: generating message( "<< count << ") :" << endl;
          out3[0] = tt_raw_header("ATU-Test",false, 2);
          cout << out3[0] << endl;
          break;

        case 1 :
          // generate a dummy data transmit token
          cout << name() << " alias BTH/GRHGen: generating message( "<< count << ") :" << endl;
          
          tt_data tt(17, false);
          ts_scaga_buf x = { 256, 512, 30};
          tt.buffer = x;
          
          out3[0] = tt;
          cout << out3[0] << endl;
          break;
      }
      count++;
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
  
    // received messages counter
    int count;
    
    void print1() {
      cout << name() << ": got message(" << count++ << ") for MFETCH: " << endl;
      cout << in1[0] << endl;

      if( count > NUM_TEST_MSGS ) sc_stop();  
    }
    
    void print2() {
      cout << name() << ": got message(" << count++ << ") for MSTORE: " << endl;
      cout << in2[0] << endl;
      
      if( count > NUM_TEST_MSGS ) sc_stop();  
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
: public smoc_graph {
  public:
    m_top( sc_module_name name )
      : smoc_graph(name)
    {
      m_source      &src = registerNode(new m_source("src"));
      
      m_sink        &snk = registerNode(new m_sink("snk"));
      
      ib_m_atu      &atu = registerNode(new ib_m_atu("atu",cout));

      assert( dynamic_cast<smoc_root_node *>(&atu) != NULL );
      
      connectNodePorts( src.out1, atu.in_rq2atu );
      connectNodePorts( src.out2, atu.in_tq2atu );
      connectNodePorts( src.out3, atu.in_bth_grh_gen2atu );
      
      connectNodePorts( atu.out_atu2mfetch, snk.in1 );
      connectNodePorts( atu.out_atu2mstore, snk.in2 );
    }
};

int sc_main (int argc, char **argv) {
  smoc_top_moc<m_top> top("top");
  
  sc_start(-1);
  return 0;
}
