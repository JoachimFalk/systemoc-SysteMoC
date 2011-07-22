#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_moc.hpp>

//#  include <CoSupport/SystemC/PerformanceEvaluation.hpp>
//#  include <CoSupport/Tracing/TracingFactory.hpp>

class SimpleSink: public smoc_actor{
public:
  smoc_port_in<int> in;
  
  SimpleSink(sc_module_name name,  sc_module_name id_in)
    : smoc_actor(name, start), in(id_in)
  {
    
    start =(in(1))   >> 
      CALL(SimpleSink::process)               >>
      start;

  }
  
private:

  void process() {
    std::cout << name() << "  recv " << in[0]
              << " @ " << sc_time_stamp() << std::endl;
  }
  
  smoc_firing_state start;
};

