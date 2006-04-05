#include <iostream>

#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_fifo.hpp>
#include <smoc_node_types.hpp>

//#include "examplenetworkpacket.hh"

#include "producermodule.hpp"
#include "consumermodule.hpp"
#include "channel.hpp"

#include "cryptomodule.hpp"

/**
 * Actor-graph of crypto example
 */
class CryptoExample : public smoc_graph{

  private:

    ProducerModule pModule;
    ConsumerModule cModule;
    CryptoModule pEncModule, pDecModule, cEncModule, cDecModule;
    MD5 pMd5_sign, pMd5_check, cMd5_sign, cMd5_check;
    Channel channel;
    
  public:

    CryptoExample(sc_module_name name, const char* input)
      : smoc_graph(name),
        pModule("producer", input),
        cModule("consumer"),
        pEncModule("pEncModule"),
        pDecModule("pDecModule"),
        cEncModule("cEncModule"),
        cDecModule("cDecModule"),
        pMd5_sign("pMd5_sign"),
        pMd5_check("pMd5_check"),
        cMd5_sign("cMd5_sign"),
        cMd5_check("cMd5_check"),
        channel("channel")
        {

          connectNodePorts(pModule.out, pEncModule.in);
          connectNodePorts(pEncModule.out, pMd5_sign.in);
          connectNodePorts(pMd5_sign.out, channel.in1);
          connectNodePorts(channel.out1, cMd5_check.in);
          connectNodePorts(cMd5_check.out, cDecModule.in);
          connectNodePorts(cDecModule.out, cModule.in);
       
          connectNodePorts(cModule.out, cEncModule.in);
          connectNodePorts(cEncModule.out, cMd5_sign.in);
          connectNodePorts(cMd5_sign.out, channel.in2);
          connectNodePorts(channel.out2, pMd5_check.in);
          connectNodePorts(pMd5_check.out, pDecModule.in);
          connectNodePorts(pDecModule.out, pModule.in);

        }
};

int sc_main (int argc, char **argv) {

#define GENERATE "--generate-problemgraph"
  const char* input;
 
  if(argc == 1){
    std::cerr << "Please specify input file for producer!\nsimulation-crypto [input-file]" << std::endl;
  } 
  if(argc > 1 && 0 != strncmp(argv[1], GENERATE, sizeof(GENERATE))){
    input = argv[1];
  }

  smoc_top_moc<CryptoExample> cryptoexample("crypto_example", input);
  
  if (argc > 1 && 0 == strncmp(argv[1], GENERATE, sizeof(GENERATE))) {
    smoc_modes::dump(std::cout, cryptoexample);
  } else {
    sc_start(-1);
  }
#undef GENERATE
  return 0;
}

