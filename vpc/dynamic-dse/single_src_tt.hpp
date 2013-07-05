#include <cstdlib>
#include <iostream>

#define PERFORMANCE_EVALUATION

#ifdef PERFORMANCE_EVALUATION
//#  include <CoSupport/SystemC/PerformanceEvaluation.hpp>
#  include <CoSupport/Tracing/TracingFactory.hpp>
#endif // PERFORMANCE_EVALUATION


#include <systemoc/smoc_tt.hpp>

class SingleSource_tt: public smoc_periodic_actor{
public:
  smoc_port_out<sc_time> out;
  
  SingleSource_tt(sc_module_name name, sc_time per, sc_time off, float jitter, sc_module_name id_out, bool b_evaluate = false, std::string traceName = "trace")
    : smoc_periodic_actor(name, start, per, off, jitter), out(id_out), evaluate(b_evaluate)
  {
    if(b_evaluate){
      trace_object = ((CoSupport::Tracing::PtpTracer::Ptr)CoSupport::Tracing::TracingFactory::getInstance().createPtpTracer(traceName));
    }
    start = (out(1))                 >> 
      CALL(SingleSource_tt::process)           >>
      start;
  }

private:
  CoSupport::Tracing::PtpTracer::Ptr trace_object;
  void process() {
  //  this->resetEvent();
   if(evaluate){
      #ifdef PERFORMANCE_EVALUATION
        //CoSupport::SystemC::PerformanceEvaluation::getInstance().startUnit();
        trace_object->start();
      #endif // PERFORMANCE_EVALUATION
    }
 //   std::cout << name() << "  send: " << sc_time_stamp() << std::endl;
    out[0]=sc_time_stamp();
  }
  bool evaluate;
  smoc_firing_state start;

};
