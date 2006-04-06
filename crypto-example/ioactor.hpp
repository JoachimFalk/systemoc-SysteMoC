#ifndef IOACTOR_HPP_
#define IOACTOR_HPP_

#include <iostream>

#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_node_types.hpp>

#include "helper.hpp"
#include "examplenetworkpacket.hpp"

/**
 * Simple dummy actor to just foward packets through channel
 */
class IOActor : public smoc_actor{

  private:

    smoc_firing_state start;
    
    virtual void forwardPacket(){
      ExampleNetworkPacket packet;
      packet = in[0];
    
      printMsg(packet);    
   
      out[0] = packet;
    }
  
    void printMsg(ExampleNetworkPacket &packet){
      if(packet.processing_request != ExampleNetworkPacket::PR_set_key){

         std::cout << "ioactor " << this->basename() << "> Received new package with data:" << std::endl;
         std::cout << "\"";
         Helper::Datachars data;
         for(int i=0; i < packet.getUsedPayload(); i++){
           Helper::datawordToString(packet.payload[i], data);
           for(int j=0; j < BYTES_PER_DATAWORD; j++){
             std::cout << data.position[j];
           }
         }
         std::cout << "\"" <<  std::endl;
      } 
    }
     
  public:

    smoc_port_in< ExampleNetworkPacket > in;
    smoc_port_out< ExampleNetworkPacket > out;

    IOActor(sc_module_name name) : smoc_actor(name, start){

      start = (in(1) && out(1)) >> CALL(IOActor::forwardPacket) >> start;

    }
};

#endif //IOACTOR_HPP_
