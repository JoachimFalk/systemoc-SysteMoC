#include <iostream>

#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_fifo.hpp>
#include <smoc_node_types.hpp>

#include <memory_interface.h>


using Expr::field;
using std::endl;
using std::cout;

class m_source: public smoc_actor {
public:
  smoc_port_out<data_buffer>   out;
private: 
  int times;
  smoc_firing_state start;
  bool xTimes() const{
    return (times!=0);
  }
  void action(){
    out[0]=data_buffer("123456789abcdefghijkl", times, 20*times);
    times--;
  }
public:
  m_source(sc_module_name name):smoc_actor(name,start),times(2){
    start= (out(1) && guard(&m_source::xTimes) )>> CALL(m_source::action) >> start;
  }
 
};


class m_sink: public smoc_actor {
public:
  smoc_port_in<storage_completed_msg>   in;
private: 
  smoc_firing_state start;
  void action(){
    cerr << "SINK> got storage_completed_msg: " << in[0].buffer_id<< endl;
  }
public:
  m_sink(sc_module_name name):smoc_actor(name,start){
    start= (in(1) )>> CALL(m_sink::action) >> start;
  }
 
};






class m_top : public smoc_graph {
public:
  m_top(sc_module_name name): smoc_graph(name){
      char buf1[512]="a0a1a2a3a4a5a6a7a8a9b0b1b2b3b4b5b6b78b9c0c1c23c4c5c6c78c9d0d1d2d3d4d5d6d7d8d9e0e1e2e3e4e5e6e7e8e9f0f1f2f3f4f5f6f7f8f9g0g1g2g3g4g5g6g7g8g9h0h1h2h3h4h5h6h7h8h9i0i1i2i3i4i5i6i7i8i9j0j1j2j3j45j6j7j8j9k0k12k3k4k5k6k7k8k9l0l1l2l3l4l5l6l7l8l9m0m1m2m3m4m5m6m7m8m9n0n1n2n3n4n5n6n7n8n";
      char buf2[512]="0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
      
      //memory_access_read<16>  &read   = registerNode(new memory_access_read<16>("read",buf1));
      memory_access_write<16> &write  = registerNode(new memory_access_write<16>("write",buf2));

      m_source                &source = registerNode(new m_source("source"));
      m_sink                  &sink   = registerNode(new m_sink("sink"));
      connectNodePorts( source.out,  write.in );  
      connectNodePorts( write.out, sink.in ); 
    
  }
};
  

int sc_main (int argc, char **argv) {
  smoc_top_moc<m_top> top("top");
  
#define GENERATE "--generate-problemgraph"
  if(argc > 1 && 0==strncmp(argv[1],GENERATE,sizeof(GENERATE))){
    smoc_modes::dump(std::cout, top);
  }else{  
    sc_start(-1);
  }
#undef GENERATE

  return 0;
}     
 
