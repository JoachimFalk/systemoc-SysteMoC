#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_moc.hpp>

class SimpleTask2_2: public smoc_actor{
public:
  smoc_port_in<sc_time> in1, in2;
  smoc_port_out<sc_time> out1, out2;
  
  SimpleTask2_2(sc_module_name name, sc_module_name id_in1, sc_module_name id_in2, sc_module_name id_out1, sc_module_name id_out2)
    : smoc_actor(name, start), in1(id_in1), in2(id_in2), out1(id_out1), out2(id_out2)
  {
    start =(in1(1) && in2(1))   >>
           (out1(1) && out2(1))  >>
           CALL(SimpleTask2_2::process)  >>
           start;
  }
  
private:
  void process() {
    //std::cout <<  name() << " recv/processed/sended " << in1[0]
  //            << " @ " << sc_time_stamp() << std::endl;
    out1[0] = in1[0];
    out2[0] = in1[0];
  }
  
  smoc_firing_state start;
};

