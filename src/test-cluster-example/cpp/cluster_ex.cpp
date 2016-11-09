
#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_graph.hpp>

#include "DynSwitch.hpp"
#include "StaticCluster.hpp"

#ifndef KASCPAR_PARSING    
using namespace std;
#endif

template<int upsample_factor>
class m_top
  : public smoc_graph
{
private:
  StaticCluster<upsample_factor> static_cluster;
  DynSwitch dyn_switch;

public:
  m_top<upsample_factor>(sc_core::sc_module_name name)
    : smoc_graph(name),
      static_cluster("static_cluster",5,10),
      dyn_switch("dyn_switch",1000)
  {
    
    connectNodePorts<2>(dyn_switch.out1,static_cluster.in1);
    connectNodePorts<1>(dyn_switch.out2,static_cluster.in2);

    connectNodePorts<2>(static_cluster.out1,dyn_switch.in1);
    connectNodePorts<1>(static_cluster.out2,dyn_switch.in2);
  }

};



int sc_main (int argc, char **argv) {
#ifndef KASCPAR_PARSING    
  smoc_top_moc<m_top<10> > top("top");
#endif
  sc_core::sc_start();
  return 0;
}
