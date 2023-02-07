// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_multicast_sr_signal.hpp>

#include "ConstSource.hpp"
#include "NonStrictAnd.hpp"
#include "TerminalSink.hpp"

class NonStrictAndTestBench
  :public smoc_graph_sr {
public:
  NonStrictAndTestBench(sc_module_name name, int times )
    :smoc_graph_sr(name),
     snk("Sink", std::string("\t") ),
     nsAnd("firstNSAnd"),
     nsAnd2("secondNSAnd"),
     src0("Source1", 1, times),
     src1("Source2", 2, times),
     src2("Source3", 1, times)
  {
    SMOC_REGISTER_CPARAM(times);

    connectNodePorts(src0.out,   nsAnd.op0,  smoc_multicast_sr_signal<int>());
    connectNodePorts(src1.out,   nsAnd.op1,  smoc_multicast_sr_signal<int>());
    connectNodePorts(src2.out,   nsAnd2.op0, smoc_multicast_sr_signal<bool>());
    connectNodePorts(nsAnd.out,  nsAnd2.op1, smoc_multicast_sr_signal<bool>());
    connectNodePorts(nsAnd2.out, snk.in,     smoc_multicast_sr_signal<bool>());
    
    // FIXME (MS) prohibit this:
    //    dummySrc.out( dummy );
    //    dummySnk.in( dummy );
  }
protected:
  TerminalSink<bool, std::string > snk;
  NonStrictAnd<int> nsAnd;
  NonStrictAnd<bool> nsAnd2;
  ConstSource<int>  src0;
  ConstSource<int>  src1;
  ConstSource<bool>  src2;
};
 

int sc_main (int argc, char **argv) {
  size_t count = (argc>1?atoi(argv[1]):0);
  smoc_top_moc<NonStrictAndTestBench> nsa_tb("top", count);
  
  sc_start();

  return 0;
}
