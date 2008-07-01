// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_multicast_sr_signal.hpp>

#include "ConstSource.hpp"
#include "NonStrictAnd.hpp"
#include "TerminalSink.hpp"

class MulticastTestBench
  :public smoc_graph_sr{
protected:
  ConstSource<bool>  zero;
  ConstSource<bool>  oneTime;
  TerminalSink<bool, std::string > directSnk;
  TerminalSink<bool, std::string > andSnk;
  NonStrictAnd<bool> nsAndZero;

public:
  MulticastTestBench(sc_module_name name, int times)
    :smoc_graph_sr(name),
     zero     ("Zero",    0, times),
     oneTime  ("OneTime", 1, 1),
     directSnk("DirectSink", std::string("direct:\t")),
     andSnk   ("AndSink",    std::string("and:\t")),
     nsAndZero("NsAndZero"){
     
    smoc_multicast_sr_signal<bool> sig1;
    smoc_multicast_sr_signal<bool> sig2;
    smoc_multicast_sr_signal<bool> sig3;
   
    sig1.connect(zero.out)
      .connect(nsAndZero.op0)
      .connect(directSnk.in);
    
    sig2.connect(oneTime.out)
      .connect(nsAndZero.op1);
    
    sig3.connect(nsAndZero.out)
      .connect(andSnk.in);
  }
};
 

int sc_main (int argc, char **argv) {
  size_t count = (argc>1?atoi(argv[1]):0);
  smoc_top_moc<MulticastTestBench> nsa_tb("top", count);
  
  sc_start();

  return 0;
}
