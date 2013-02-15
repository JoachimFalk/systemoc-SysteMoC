#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_moc.hpp>

class SimpleTask1_2: public smoc_actor{
public:
  smoc_port_in<sc_time> in;
  smoc_port_out<sc_time> out1, out2;
  
  SimpleTask1_2(sc_module_name name, sc_module_name id_in, sc_module_name id_out1, sc_module_name id_out2)
    : smoc_actor(name, start), in(id_in), out1(id_out1), out2(id_out2)
  {
    start =(in(1))   >>
           (out1(1) && out2(1))  >>
           CALL(SimpleTask1_2::process)  >>
           start;
  }
  
private:
  void process() {
   // std::cout <<  name() << " recv/processed/sended " << in[0]
   //           << " @ " << sc_time_stamp() << std::endl;
    out1[0] = in[0];
    out2[0] = in[0];
  }
  
  smoc_firing_state start;
};

