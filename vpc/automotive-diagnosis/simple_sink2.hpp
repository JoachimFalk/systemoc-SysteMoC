#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_moc.hpp>
#define PERFORMANCE_EVALUATION

#ifdef PERFORMANCE_EVALUATION
//#  include <CoSupport/SystemC/PerformanceEvaluation.hpp>
#  include <CoSupport/Tracing/TracingFactory.hpp>
#endif // PERFORMANCE_EVALUATION



class SimpleSink2: public smoc_actor{
public:
  smoc_port_in<sc_time> in1, in2;
  bool measure_once;
  
  SimpleSink2(sc_module_name name,  sc_module_name id_in1, sc_module_name id_in2, std::string trace)
    : smoc_actor(name, start), in1(id_in1), in2(id_in2)
  {
	
    trace_object = ((CoSupport::Tracing::PtpTracer::Ptr)CoSupport::Tracing::TracingFactory::getInstance().createPtpTracer(trace));
    measure_once = true;
    
    start =(in1(1) && in2(1))   >> 
      CALL(SimpleSink2::process)               >> CALL(SimpleSink2::process2) >>
      start;

  }
  
private:
  CoSupport::Tracing::PtpTracer::Ptr trace_object;
  CoSupport::Tracing::PtpTracer::Ptr trace_object2;
  void process() {
 //   std::cout << name() << "  recv " << in1[0] << " and " << in2[0]
 //             << " @ " << sc_time_stamp() << std::endl;
  }

  void process2() {
	if(measure_once){
   // 		std::cout << name() << "\t event-based\t recv completed"
  //            		  << " @ " << sc_time_stamp() << std::endl;
# ifdef PERFORMANCE_EVALUATION
    	    if(trace_object != 0) trace_object->stop();
# endif // PERFORMANCE_EVALUATION
	//	measure_once = false;
	}

  } 
  
  smoc_firing_state start;
};

