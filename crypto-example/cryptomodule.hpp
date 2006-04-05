#ifndef CRYPTO_MODULE_HPP
#define CRYPTO_MODULE_HPP

#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_fifo.hpp>
#include <smoc_node_types.hpp>

#include "dispatcher.hpp"
#include "merger.hpp"

#include "cryptoalgorithm.hpp"
#include "blowfish.hpp"
#include "des3.hpp"
#include "md5.hpp"

#include "examplenetworkpacket.hpp"

class CryptoModule : public smoc_graph{

  private:

    DES3 des;
    Blowfish bf;
    Dispatcher dispatcher;
    Merger merger;
    
  public:

    smoc_port_in< ExampleNetworkPacket > in;
    smoc_port_out< ExampleNetworkPacket > out;

    CryptoModule(sc_module_name name) 
      : smoc_graph(name),
        des("DES3"),
        bf("Blowfish"),
        dispatcher("Dispatcher"),
        merger("Merger")
        {

          // establish connection to interface ports
          connectInterfacePorts(in, dispatcher.in);
          connectInterfacePorts(out, merger.out);

          // establish connection btw actor in subgraph
          connectNodePorts(dispatcher.out_des3, des.in);
          connectNodePorts(dispatcher.out_blowfish, bf.in);
          
          connectNodePorts(des.out, merger.in_des3);
          connectNodePorts(bf.out, merger.in_blowfish);
          
        }
};

#endif // CRYPTO_MODULE_HPP_
