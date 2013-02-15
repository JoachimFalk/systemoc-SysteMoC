#include <cstdlib>
#include <iostream>

#define PERFORMANCE_EVALUATION

#ifdef PERFORMANCE_EVALUATION
//#  include <CoSupport/SystemC/PerformanceEvaluation.hpp>
#  include <CoSupport/Tracing/TracingFactory.hpp>
#endif // PERFORMANCE_EVALUATION


#include <systemoc/smoc_tt.hpp>

class SimpleSource_tt_2: public smoc_periodic_actor{
public:
  smoc_port_out<sc_time> out;
  
  SimpleSource_tt_2(sc_module_name name, sc_time per, sc_time off, sc_module_name id_out, double jitter=0.0)
    : smoc_periodic_actor(name, start, per, off, jitter), out(id_out)
  {
    start = (out(5))                 >>
      CALL(SimpleSource_tt_2::process)           >>
      start;
  }

private:
  void process() {
  //  std::cout << name() << "  send: " << sc_time_stamp() << std::endl;
    out[0]=sc_time_stamp();
    out[1]=sc_time_stamp();
    out[2]=sc_time_stamp();
    out[3]=sc_time_stamp();
    out[4]=sc_time_stamp();
  }
  smoc_firing_state start;

};

