// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_multicast_sr_signal.hpp>

#include "ConstSource.hpp"
#include "NonStrictAnd.hpp"
#include "TerminalSink.hpp"


class BooleanAnd
  :public smoc_graph_sr{
public:
  smoc_port_in<bool>  op0;
  smoc_port_in<bool>  op1;
  smoc_port_out<bool> out;

  BooleanAnd( sc_module_name name )
    : smoc_graph_sr( name ),
      nsa ("BAnd")
  {
    op0(nsa.op0);
    op1(nsa.op1);
    out(nsa.out);
  }
private:
  NonStrictAnd<bool> nsa;
};


class MulticastTestBench
  :public smoc_graph_sr{
protected:
  ConstSource<bool>  zero;
  ConstSource<bool>  oneTime;
  TerminalSink<bool, std::string > directSnk;
  TerminalSink<bool, std::string > andSnk;
  BooleanAnd nsAndZero;

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
    
    connector(sig1)
      << zero.out
      << nsAndZero.op0
      << directSnk.in;
    
    connector(sig2)
      << oneTime.out
      << nsAndZero.op1;

    connector(sig3)
      << nsAndZero.out
      << andSnk.in;
  }
};
 

int sc_main (int argc, char **argv) {
  size_t count = (argc>1?atoi(argv[1]):0);
  smoc_top_moc<MulticastTestBench> nsa_tb("top", count);
  
#define GENERATE "--generate-problemgraph"
  if (argc > 1 && 0 == strncmp(argv[1], GENERATE, sizeof(GENERATE))) {
    smoc_modes::dump(std::cout, nsa_tb);
  } else {
    sc_start();
  }
#undef GENERATE
  return 0;
}
