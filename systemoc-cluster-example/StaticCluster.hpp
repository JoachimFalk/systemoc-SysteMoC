// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8:

#ifndef _INCLUDED_CLUSTER
#define _INCLUDED_CLUSTER

#ifndef XILINX_EDK_RUNTIME
#include <sstream>
#endif

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_node_types.hpp>
#ifndef __SCFE__
//# include <smoc_scheduler.hpp>
# include <systemoc/smoc_pggen.hpp>
#endif
#include <systemoc/smoc_graph_type.hpp>

#include "Upsample.hpp"
#include "Downsample.hpp"
#include "Actor_a1.hpp"
#include "Actor_a2.hpp"
#include "Actor_a3.hpp"
#include "Forward.hpp"

#define CLUSTERED_FIFO_SIZES


template <int upsample_factor>
class StaticCluster 
  : public smoc_graph
{

public:
  smoc_port_in<int> in1;
  smoc_port_in<int> in2;
  smoc_port_out<int> out1;
  smoc_port_out<int> out2;

private:

  Actor_a1 actor_a1;
  Actor_a2 actor_a2;
  Actor_a3 actor_a3;

  Upsample upsample1;
  Downsample downsample1;

  Upsample upsample2;
  Downsample downsample2;  

  Upsample upsample3;
  Downsample downsample3;
  
  Forward **forward1;
  Forward **forward2;
  Forward **forward3;

  const unsigned int num_dummy_actors;
  

public:
  StaticCluster<upsample_factor>( sc_module_name name,
                                  unsigned int num_dummy_actors,
                                  unsigned long heat_time)
    : smoc_graph(name),
      actor_a1("a1",heat_time),
      actor_a2("a2",heat_time),
      actor_a3("a3",heat_time),
      upsample1("upsample1",upsample_factor),
      downsample1("downsample1",upsample_factor),
      upsample2("upsample2",upsample_factor),
      downsample2("downsample2",upsample_factor),
      upsample3("upsample3",upsample_factor),
      downsample3("downsample3",upsample_factor),
      num_dummy_actors(num_dummy_actors)      
  {

    assert(num_dummy_actors >= 1);
    assert(upsample_factor >= 1);

#ifndef KASCPAR_PARSING
    forward1 = new Forward*[num_dummy_actors];
    for(unsigned int i = 0; i < num_dummy_actors; i++){
      std::stringstream actor_name;
      actor_name << "forward1_";
      actor_name << i;
      forward1[i] = new Forward(actor_name.str().c_str(),
                                heat_time);
    }

    forward2 = new Forward*[num_dummy_actors];
    for(unsigned int i = 0; i < num_dummy_actors; i++){
      std::stringstream actor_name;
      actor_name << "forward2_";
      actor_name << i;
      forward2[i] = new Forward(actor_name.str().c_str(),
                                heat_time);
    }

    forward3 = new Forward*[num_dummy_actors];
    for(unsigned int i = 0; i < num_dummy_actors; i++){
      std::stringstream actor_name;
      actor_name << "forward3_";
      actor_name << i;
      forward3[i] = new Forward(actor_name.str().c_str(),
                                heat_time);
    }




    actor_a1.in1(in2);
    connectNodePorts(actor_a1.out2,
                     actor_a2.in2,
                     smoc_fifo<int>(3)  << 0 << 1
                     );

    connectNodePorts(actor_a2.out2,
                     actor_a3.in2,
                     smoc_fifo<int>(3) << 0
                     );
    actor_a3.out1(out2);
#endif

    
    /* Connection of A2 with I1 via several dummy actors */
    upsample1.in1(in1);
#ifndef CLUSTERED_FIFO_SIZES
    connectNodePorts<upsample_factor>(upsample1.out1,
                                      (forward1[0])->in1);
#else
    connectNodePorts<2*upsample_factor>(upsample1.out1,
                                        (forward1[0])->in1);
#endif
    for(unsigned int i = 0; i < num_dummy_actors-1; i++){
#ifndef CLUSTERED_FIFO_SIZES
      connectNodePorts<1>((forward1[i])->out1,
                          (forward1[i+1])->in1);
#else
      connectNodePorts<2*upsample_factor>((forward1[i])->out1,
                                          (forward1[i+1])->in1);
#endif
    }
#ifndef CLUSTERED_FIFO_SIZES
    connectNodePorts<upsample_factor>((forward1[num_dummy_actors-1])->out1,
                                      downsample1.in1);
#else
    connectNodePorts<2*upsample_factor>((forward1[num_dummy_actors-1])->out1,
                                        downsample1.in1);
#endif
    connectNodePorts<2>(downsample1.out1,
                        actor_a2.in1);

    
    /* Connection of A2 with O1 via several dummy actors */
    connectNodePorts<2>(actor_a2.out1,
                        upsample2.in1);
#ifndef CLUSTERED_FIFO_SIZES
    connectNodePorts<upsample_factor>(upsample2.out1,
                                      (forward2[0])->in1);
#else
    connectNodePorts<2*upsample_factor>(upsample2.out1,
                                        (forward2[0])->in1);
#endif
    for(unsigned int i = 0; i < num_dummy_actors-1; i++){
#ifndef CLUSTERED_FIFO_SIZES
      connectNodePorts<1>((forward2[i])->out1,
                          (forward2[i+1])->in1);
#else
      connectNodePorts<2*upsample_factor>((forward2[i])->out1,
                                          (forward2[i+1])->in1);
#endif
    }
#ifndef CLUSTERED_FIFO_SIZES
    connectNodePorts<upsample_factor>((forward2[num_dummy_actors-1])->out1,
                                      downsample2.in1);
#else
    connectNodePorts<2*upsample_factor>((forward2[num_dummy_actors-1])->out1,
                                      downsample2.in1);
#endif
    downsample2.out1(out1);

    
    /* Connectio of A1 with A3 via several dummy actors */
    connectNodePorts<1>(actor_a1.out1,
                        upsample3.in1);
    connectNodePorts<upsample_factor>(upsample3.out1,
                                      (forward3[0])->in1);
    for(unsigned int i = 0; i < num_dummy_actors-1; i++){
#ifndef CLUSTERED_FIFO_SIZES
      connectNodePorts<1>((forward3[i])->out1,
                          (forward3[i+1])->in1);
#else
      connectNodePorts<upsample_factor>((forward3[i])->out1,
                                        (forward3[i+1])->in1);
#endif
    }
    connectNodePorts<upsample_factor>((forward3[num_dummy_actors-1])->out1,
                                      downsample3.in1);
    connectNodePorts<1>(downsample3.out1,actor_a3.in1);
    
                        
    
    
    
  }

  ~StaticCluster(){
    for(unsigned int i = 0; i < num_dummy_actors; i++){
      delete forward1[i];
      delete forward2[i];
      delete forward3[i];
    }

    delete[] forward1;
    delete[] forward2;
    delete[] forward3;
    
  }

};


#endif
