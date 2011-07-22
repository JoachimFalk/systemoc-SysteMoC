#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_tt.hpp>
#include <CoSupport/Tracing/TracingFactory.hpp>

class SendTask: public smoc_periodic_actor{
public:
  smoc_port_in<int> in;

  smoc_port_out<int> out5;
  smoc_port_out<int> out6;
  
  SendTask(sc_module_name name, sc_time per, sc_time off)
    : smoc_periodic_actor(name, start, per, off)
  {
    trace_object = ((CoSupport::Tracing::PtpTracer::Ptr)CoSupport::Tracing::TracingFactory::getInstance().createPtpTracer("message"));
    
    start =(in(6))   >>
        (out6(1))  >>
        CALL(SendTask::process_six)  >> start
        |(in(5))   >>
           (out5(1))  >>
           CALL(SendTask::process_five)  >>start;
  }
  
private:
  void process_six() {
    std::cout << name() << " recv/processed/sent six " << in[0] << " " << in[1] << " " << in[2] << " " << in[3] << " " << in[4] << " " << in[5]
              << " @ " << sc_time_stamp() << std::endl;
    out6[0] = in[0];
    out6[0] = 6;
    trace_object->start();
  }
  
  void process_five() {
      std::cout << name() << " recv/processed/sent five " << in[0] << " " << in[1] << " " << in[2] << " " << in[3] << " " << in[4] << " @ " << sc_time_stamp() << std::endl;
      out5[0] = in[0];
      out5[0] = 5;
      trace_object->start();
    }

  CoSupport::Tracing::PtpTracer::Ptr trace_object;
  smoc_firing_state start;
};

