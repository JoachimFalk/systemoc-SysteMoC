#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_moc.hpp>
#include <CoSupport/Tracing/TracingFactory.hpp>

class ReceiveTask: public smoc_actor{
public:
  smoc_port_in<int> in5;
  smoc_port_in<int> in6;
  smoc_port_out<int> out;
  
  ReceiveTask(sc_module_name name, sc_module_name id_in5, sc_module_name id_in6, sc_module_name id_out)
    : smoc_actor(name, start), in5(id_in5), in6(id_in6), out(id_out)
  {
    trace_object = ((CoSupport::Tracing::PtpTracer::Ptr)CoSupport::Tracing::TracingFactory::getInstance().createPtpTracer("message"));

    start =(in5(1))   >>
           (out(5))  >>
           CALL(ReceiveTask::process_five)  >>
           start
           | (in6(1)) >>
           (out(6))  >>
           CALL(ReceiveTask::process_six)  >>
           start;
  }
  
private:

  void process_five() {
    std::cout <<  name() << " process_five recv/processed/sent " << in5[0]
              << " @ " << sc_time_stamp() << std::endl;
    out[0] = in5[0];
    out[1] = in5[0];
    out[2] = in5[0];
    out[3] = in5[0];
    out[4] = in5[0];
    trace_object->stop();
  }
  
  void process_six() {
      std::cout <<  name() << " process_six recv/processed/sent " << in6[0]
                << " @ " << sc_time_stamp() << std::endl;
      out[0] = in6[0];
      out[1] = in6[0];
      out[2] = in6[0];
      out[3] = in6[0];
      out[4] = in6[0];
      out[5] = in6[0];
      trace_object->stop();
    }

  void process_unknown() {
      std::cout <<  name() << " process_unknown recv/processed/sent " << in5[0]
                << " @ " << sc_time_stamp() << std::endl;
      out[0] = in5[0];
      trace_object->stop();
    }

  CoSupport::Tracing::PtpTracer::Ptr trace_object;
  smoc_firing_state start;
};

