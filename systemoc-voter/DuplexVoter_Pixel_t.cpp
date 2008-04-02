// vim: set sw=2 ts=8:

#include "channels.hpp"
#include "DuplexVoter_Pixel_t.hpp"


void DuplexVoter::forward_data() {
    out[0] = in1[0];
    outTimeOut[0] = 1;
  }

void DuplexVoter::failed_action() {
  int i = inTimeOut[0];
  }

DuplexVoter::DuplexVoter( sc_module_name name )
     : smoc_actor(name,start)
  {
    start = (in1(1) && in2(1) && (in1.getValueAt(0) == in2.getValueAt(0))) 
         >> (out(1) && outTimeOut(1)) 
         >> CALL(DuplexVoter::forward_data) 
         >> start
       | inTimeOut(1) >> CALL(DuplexVoter::failed_action) 
         >> failed
       | (in1(1) && in2(1) && (in1.getValueAt(0) != in2.getValueAt(0)))
         >> failed;
  }

class m_source_Pixel_t: public smoc_actor {
  public:
    smoc_port_out<Pixel_t> out;
  private:
    
    void process() {
      out[0] = 0;
    }

    smoc_firing_state start;
  public:
    m_source_Pixel_t( sc_module_name name )
      :smoc_actor( name, start ) {
      start =  out(1) >> CALL(m_source_Pixel_t::process) >> start;
    }
};

class m_sink_Pixel_t: public smoc_actor {
  public:
    smoc_port_in<Pixel_t> in;
  private:
    void process() {
    }
    
    smoc_firing_state start;
  public:
    m_sink_Pixel_t( sc_module_name name )
      :smoc_actor( name, start ) {
      start = in(1) >> CALL(m_sink_Pixel_t::process) >> start;
    }
};

class m_source_bool: public smoc_actor {
  public:
    smoc_port_out<int> out;
  private:
    void process() {
      out[0] = 1;
    }
    
    smoc_firing_state start;
  public:
    m_source_bool( sc_module_name name )
      :smoc_actor( name, start ) {
      start = out(0,1) >> CALL(m_source_bool::process) >> start;
    }
};

class m_sink_bool: public smoc_actor {
  public:
    smoc_port_in<int> in;
  private:
    void process() {
    }
    
    smoc_firing_state start;
  public:
    m_sink_bool( sc_module_name name )
      :smoc_actor( name, start ) {
      start = in(1) >> CALL(m_sink_bool::process) >> start;
    }
};

class DuplexTester: public smoc_graph {

public:

  DuplexTester(sc_module_name name) : smoc_graph(name), voter("Voter"), src_bool("Source_Bool"), src_1("Source_1"), src_2("Source_2"), sink_bool("Sink_Bool"), sink("Sink") {
    connectNodePorts<2>(src_bool.out, voter.inTimeOut);
    connectNodePorts<2>(src_1.out, voter.in1);
    connectNodePorts<2>(src_2.out, voter.in2);
    connectNodePorts<2>(voter.outTimeOut, sink_bool.in);
    connectNodePorts<2>(voter.out, sink.in);
  }
  
private:

  m_source_bool src_bool;
  m_source_Pixel_t src_1;
  m_source_Pixel_t src_2;
  m_sink_bool sink_bool;
  m_sink_Pixel_t sink;
  DuplexVoter voter;

};

int sc_main (int argc, char **argv) {
    
  smoc_top_moc<DuplexTester> tester("Tester");
  
  sc_start();
  
  return 0;
}

