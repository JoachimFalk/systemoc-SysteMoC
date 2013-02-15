#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_moc.hpp>

class SimpleTask: public smoc_actor{
public:
  smoc_port_in<sc_time> in;
  smoc_port_out<sc_time> out;
  
  SimpleTask(sc_module_name name, sc_module_name id_in, sc_module_name id_out)
    : smoc_actor(name, start), in(id_in), out(id_out)
  {
    start =(in(1))   >>
           (out(1))  >>
           CALL(SimpleTask::process)  >>
           start;
  }
  
private:
  void process() {
   // std::cout <<  name() << " recv/processed/sended " << in[0]
   //           << " @ " << sc_time_stamp() << std::endl;
    out[0] = in[0];
  }
  
  smoc_firing_state start;
};

class SimpleTask_4t: public smoc_actor{
public:
  smoc_port_in<sc_time> in;
  smoc_port_out<sc_time> out;

  SimpleTask_4t(sc_module_name name, sc_module_name id_in, sc_module_name id_out)
    : smoc_actor(name, start), in(id_in), out(id_out)
  {
    start =(in(1))   >>
           (out(4))  >>
           CALL(SimpleTask_4t::process)  >>
           start;
  }

private:
  void process() {
    //std::cout <<  name() << " recv/processed/sended " << in[0]
   //           << " @ " << sc_time_stamp() << std::endl;
    out[0] = in[0];
  }

  smoc_firing_state start;
};
