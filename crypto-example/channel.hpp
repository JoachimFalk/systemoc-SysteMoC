#ifndef CHANNEL_HPP_
#define CHANNEL_HPP_

#include <smoc_fifo.hpp>
#include <smoc_node_types.hpp>
#include "ioactor.hpp"

/**
 * Simple channel for bi-directional communication
 */
class Channel : public smoc_graph{
  
  private:
    IOActor io1;
    IOActor io2;
    
  public:

    smoc_port_in< ExampleNetworkPacket > in1, in2;
    smoc_port_out< ExampleNetworkPacket > out1, out2;

    Channel(sc_module_name name, int fail_rate=0) 
      : smoc_graph(name),
        io1("io1"),
        io2("io2")
      {

          // establish connection to interface ports
          connectInterfacePorts(in1, io1.in);
          connectInterfacePorts(out1, io1.out);
          connectInterfacePorts(in2, io2.in);
          connectInterfacePorts(out2, io2.out);
        
        }

};

#endif //CHANNEL_HPP_
