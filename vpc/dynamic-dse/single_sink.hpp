#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_moc.hpp>

//#  include <CoSupport/SystemC/PerformanceEvaluation.hpp>
#  include <CoSupport/Tracing/TracingFactory.hpp>

class SingleSink: public smoc_actor{
public:
  smoc_port_in<sc_time> in;
  
  SingleSink(sc_module_name name,  sc_module_name id_in, std::string trace = "")
    : smoc_actor(name, start), in(id_in)
  {
    if(trace != "")
    trace_object = ((CoSupport::Tracing::PtpTracer::Ptr)CoSupport::Tracing::TracingFactory::getInstance().createPtpTracer(trace));
    
    start =(in(1))   >> 
      CALL(SingleSink::process)               >>
      start;

  }
  
private:

  CoSupport::Tracing::PtpTracer::Ptr trace_object;
  void process() {
    std::cout << name() << "  recv " << in[0]
              << " @ " << sc_time_stamp() << std::endl;
    if(trace_object != 0) trace_object->stop();

  }
  
  smoc_firing_state start;
};
