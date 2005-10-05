
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
    
#include "Add.hpp"
#include "Scale.hpp"
#include "Delay.hpp"
    
class m_biquad_test : public smoc_graph {
  public:
  smoc_port_in<int>INPUT;
  smoc_port_out<int>Output;
  m_biquad_test( sc_module_name name )
    : smoc_graph(name)
    {
      m_Add &Add1 = registerNode(new m_Add("Add1",16));
      m_Add &Add2 = registerNode(new m_Add("Add2",16));
      m_Add &Add3 = registerNode(new m_Add("Add3",16));
      m_Add &Add4 = registerNode(new m_Add("Add4",16));
      m_Scale &Scale1 = registerNode(new m_Scale("Scale1",16,1));
      m_Scale &Scale2 = registerNode(new m_Scale("Scale2",16,1));
      m_Scale &Scale4 = registerNode(new m_Scale("Scale4",16,1));
      m_Scale &Scale3 = registerNode(new m_Scale("Scale3",16,1));
      m_Delay &Delay1 = registerNode(new m_Delay("Delay1",16,0));
      m_Delay &Delay2 = registerNode(new m_Delay("Delay2",16,0));
      m_Delay &Delay6 = registerNode(new m_Delay("Delay6",16,0));
      m_Delay &Delay4 = registerNode(new m_Delay("Delay4",16,0));
      m_Delay &Delay5 = registerNode(new m_Delay("Delay5",16,0));
      m_Delay &Delay3 = registerNode(new m_Delay("Delay3",16,0));
      connectNodePorts(Scale2.OUT,Add2.A);
      connectNodePorts(Add2.OUT,Add1.B);
      connectNodePorts(Delay6.OUT,Add2.B);
      connectNodePorts(Scale3.OUT,Delay6.IN);
      connectNodePorts(Scale1.OUT,Delay3.IN);
      connectNodePorts(Delay3.OUT,Add4.A);
      connectNodePorts(Add4.OUT,Add3.B);
      connectNodePorts(Delay1.OUT,Add3.A);
      connectNodePorts(Delay5.OUT,Add4.B);
      connectNodePorts(Scale4.OUT,Delay5.IN);
      connectNodePorts(Delay4.OUT,Scale4.IN);
      connectNodePorts(Add1.OUT,Delay1.IN);
      connectNodePorts(Add1.OUT,Delay2.IN);
      connectNodePorts(Delay2.OUT,Scale1.IN);
      connectNodePorts(Delay2.OUT,Scale2.IN);
      connectNodePorts(Delay2.OUT,Scale3.IN);
      connectNodePorts(Delay2.OUT,Delay4.IN);
      connectInterfacePorts(INPUT,Add1.A);
      connectInterfacePorts(Add3.OUT,Output);
}
};
    
int sc_main (int argc, char **argv) {
  smoc_top_moc<m_biquad_test> top("top");
  
#define GENERATE "--generate-problemgraph"
  if (argc > 1 && 0 == strncmp(argv[1], GENERATE, sizeof(GENERATE))) {
     smoc_modes::dump(std::cout, top);
  } else { 
    sc_start(-1);
  }
#undef GENERATE
  return 0;
}