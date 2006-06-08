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

#define FIFO_SIZE 500
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

    CryptoExample(sc_module_name name, const char* input, int run)
      : smoc_graph(name),
        pModule("producer", input, run),
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

          connectNodePorts<FIFO_SIZE>(pModule.out, pEncModule.in);
          connectNodePorts<FIFO_SIZE>(pEncModule.out, pMd5_sign.in);
          connectNodePorts<FIFO_SIZE>(pMd5_sign.out, channel.in1);
          connectNodePorts<FIFO_SIZE>(channel.out1, cMd5_check.in);
          connectNodePorts<FIFO_SIZE>(cMd5_check.out, cDecModule.in);
          connectNodePorts<FIFO_SIZE>(cDecModule.out, cModule.in);
       
          connectNodePorts<FIFO_SIZE>(cModule.out, cEncModule.in);
          connectNodePorts<FIFO_SIZE>(cEncModule.out, cMd5_sign.in);
          connectNodePorts<FIFO_SIZE>(cMd5_sign.out, channel.in2);
          connectNodePorts<FIFO_SIZE>(channel.out2, pMd5_check.in);
          connectNodePorts<FIFO_SIZE>(pMd5_check.out, pDecModule.in);
          connectNodePorts<FIFO_SIZE>(pDecModule.out, pModule.in);

        }
};

int sc_main (int argc, char **argv) {

#define GENERATE "--generate-problemgraph"
  const char* input;
  int run=1;
  
  if(argc == 1){
    std::cerr << "Please specify input file for producer!\nsimulation-crypto [input-file]" << std::endl;
  } 
  if(argc > 1 && 0 != strncmp(argv[1], GENERATE, sizeof(GENERATE))){
    input = argv[1];
  }
  if(argc > 2){
    run = atoi(argv[2]);
  }
  smoc_top_moc<CryptoExample> cryptoexample("crypto_example", input, run);
  
  if (argc > 1 && 0 == strncmp(argv[1], GENERATE, sizeof(GENERATE))) {
    smoc_modes::dump(std::cout, cryptoexample);
  } else {
    sc_start(-1);
  }
#undef GENERATE
  return 0;
}

