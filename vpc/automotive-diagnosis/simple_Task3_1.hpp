#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_moc.hpp>

class SimpleTask3_1: public smoc_actor{
public:
  smoc_port_in<sc_time> in1, in2, in3;
  smoc_port_out<sc_time> out;
  
  SimpleTask3_1(sc_module_name name,  sc_module_name id_in1,  sc_module_name id_in2, sc_module_name id_in3, sc_module_name id_out)
    : smoc_actor(name, start), in1(id_in1), in2(id_in2), in3(id_in3), out(id_out)
  {
    start =(in1(1) && in2(1) && in3(1))   >>
           (out(1))  >>
           CALL(SimpleTask3_1::process)  >>
           start;
  }
  
private:
  void process() {
    //std::cout <<  name() << " recv/processed/sended " << in1[0]
  //            << " @ " << sc_time_stamp() << std::endl;
    out[0] = in1[0];
  }
  
  smoc_firing_state start;
};

