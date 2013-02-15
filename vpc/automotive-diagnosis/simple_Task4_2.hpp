#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_moc.hpp>

class SimpleTask4_2: public smoc_actor{
public:
  smoc_port_in<sc_time> in1, in2, in3, in4;
  smoc_port_out<sc_time> out1, out2;
  
  SimpleTask4_2(sc_module_name name, sc_module_name id_in1, sc_module_name id_in2, sc_module_name id_in3, sc_module_name id_in4, sc_module_name id_out1, sc_module_name id_out2)
    : smoc_actor(name, start), in1(id_in1), in2(id_in2), in3(id_in3), in4(id_in4), out1(id_out1), out2(id_out2)
  {
    start =(in1(1) && in2(1) && in3(1) && in4(1))   >>
           (out1(1) && out2(1))  >>
           CALL(SimpleTask4_2::process)  >>
           start;
  }
  
private:
  void process() {
  //  std::cout <<  name() << " recv/processed/sended " << in1[0]
  //            << " @ " << sc_time_stamp() << std::endl;
    out1[0] = in1[0];
    out2[0] = in1[0];
  }
  
  smoc_firing_state start;
};

