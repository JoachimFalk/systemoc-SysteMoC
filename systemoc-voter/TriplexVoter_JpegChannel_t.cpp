// vim: set sw=2 ts=8:

#include "channels.hpp"
#include "TriplexVoter_JpegChannel_t.hpp"


void TriplexVoter::forward_in1() {
    out[0] = in1[0];
    outTimeOut[0] = 1;
}

void TriplexVoter::forward_in2() {
    out[0] = in2[0];
    outTimeOut[0] = 1;
}

void TriplexVoter::failed_action() {
  int i = inTimeOut[0];
  outTimeOut[0] = 1;
}

TriplexVoter::TriplexVoter( sc_module_name name )
     : smoc_actor(name,start)
  {
    start = (in1(1) && in2(1) && in3(1) && (in1.getValueAt(0) == in2.getValueAt(0))) 
         >> (out(1) && outTimeOut(1)) 
         >> CALL(TriplexVoter::forward_in1) 
         >> start
       |    (in1(1) && in2(1) && in3(1) && (in1.getValueAt(0) == in3.getValueAt(0)))
         >> (out(1) && outTimeOut(1)) 
         >> CALL(TriplexVoter::forward_in1) 
         >> start
       |    (in1(1) && in2(1) && in3(1) && (in2.getValueAt(0) == in3.getValueAt(0)))
         >> (out(1) && outTimeOut(1)) 
         >> CALL(TriplexVoter::forward_in2) 
         >> start
       |    (in1(1) && in2(1) && in3(1) && (in1.getValueAt(0) != in2.getValueAt(0)) && 
            (in1.getValueAt(0) != in3.getValueAt(0)) && (in2.getValueAt(0) != in3.getValueAt(0)))
         >> failed
       |    inTimeOut(1)
         >> outTimeOut(1)
         >> CALL(TriplexVoter::failed_action)
         >> timeOut;
     timeOut = (in1(1) && in2(1) && (in1.getValueAt(0) == in2.getValueAt(0)))
            >> (out(1) && outTimeOut(1))
            >> CALL(TriplexVoter::forward_in1)
            >> start
          |    (in1(1) && in2(1) && (in1.getValueAt(0) != in2.getValueAt(0))) 
            >> failed
          |    (in1(1) && in3(1) && (in1.getValueAt(0) == in3.getValueAt(0)))
            >> (out(1) && outTimeOut(1))
            >> CALL(TriplexVoter::forward_in1)
            >> start
          |    (in1(1) && in3(1) && (in1.getValueAt(0) != in3.getValueAt(0))) 
            >> failed
          |    (in2(1) && in3(1) && (in2.getValueAt(0) == in3.getValueAt(0)))
            >> (out(1) && outTimeOut(1))
            >> CALL(TriplexVoter::forward_in2)
            >> start
          |    (in2(1) && in3(1) && (in2.getValueAt(0) != in3.getValueAt(0))) 
            >> failed
          |    inTimeOut(1)
            >> 	failed;

  }

class m_source_JpegChannel_t: public smoc_actor {
  public:
    smoc_port_out<JpegChannel_t> out;
  private:
    
    void process() {
      out[0] = 0;
    }

    smoc_firing_state start;
  public:
    m_source_JpegChannel_t( sc_module_name name )
      :smoc_actor( name, start ) {
      start =  out(1) >> CALL(m_source_JpegChannel_t::process) >> start;
    }
};

class m_sink_JpegChannel_t: public smoc_actor {
  public:
    smoc_port_in<JpegChannel_t> in;
  private:
    void process() {
    }
    
    smoc_firing_state start;
  public:
    m_sink_JpegChannel_t( sc_module_name name )
      :smoc_actor( name, start ) {
      start = in(1) >> CALL(m_sink_JpegChannel_t::process) >> start;
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


class TriplexTester: public smoc_graph {

public:
  TriplexTester(sc_module_name name) : smoc_graph(name), voter("Voter"), src_bool("Source_Bool"), src_1("Source_1"), src_2("Source_2"), src_3("Source_3"), sink_bool("Sink_Bool"), sink("Sink") {
    connectNodePorts<2>(src_bool.out, voter.inTimeOut);
    connectNodePorts<2>(src_1.out, voter.in1);
    connectNodePorts<2>(src_2.out, voter.in2);
    connectNodePorts<2>(src_3.out, voter.in3);
    connectNodePorts<2>(voter.outTimeOut, sink_bool.in);
    connectNodePorts<2>(voter.out, sink.in);
  }
  
private:

  m_source_bool src_bool;
  m_source_JpegChannel_t src_1;
  m_source_JpegChannel_t src_2;
  m_source_JpegChannel_t src_3;
  m_sink_bool sink_bool;
  m_sink_JpegChannel_t sink;
  TriplexVoter voter;


};

int sc_main (int argc, char **argv) {
    
  smoc_top_moc<TriplexTester> tester("Tester");
  
  sc_start();
  
  return 0;
}

