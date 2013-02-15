#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_moc.hpp>

class SimpleTask2_1: public smoc_actor{
public:
  smoc_port_in<sc_time> in1, in2;
  smoc_port_out<sc_time> out;
  
  SimpleTask2_1(sc_module_name name, sc_module_name id_in1, sc_module_name id_in2, sc_module_name id_out)
    : smoc_actor(name, start), in1(id_in1), in2(id_in2), out(id_out)
  {
    start =(in1(1) && in2(1))   >>
           (out(1))  >>
           CALL(SimpleTask2_1::process)  >>
           start;
  }
  
private:
  void process() {
   // std::cout <<  name() << " recv/processed/sended " << in1[0]
   //           << " @ " << sc_time_stamp() << std::endl;
    out[0] = in1[0];
  }
  
  smoc_firing_state start;
};

class SimpleTask2_1_hd: public smoc_actor{
public:
  smoc_port_in<sc_time> in1, in2;
  smoc_port_out<sc_time> out;

  SimpleTask2_1_hd(sc_module_name name, sc_module_name id_in1, sc_module_name id_in2, sc_module_name id_out)
    : smoc_actor(name, start), in1(id_in1), in2(id_in2), out(id_out)
  {
    start =(in1(125) && in2(125))   >>
           (out(125))  >>
           CALL(SimpleTask2_1_hd::process)  >>
           start;
  }

private:
  void process() {
    std::cout <<  name() << " recv/processed/sended " << in1[0]
              << " @ " << sc_time_stamp() << std::endl;
    out[0] = in1[0];
  }

  smoc_firing_state start;
};


class SimpleTask2_1_4t: public smoc_actor{
public:
  smoc_port_in<sc_time> in1, in2;
  smoc_port_out<sc_time> out;

  SimpleTask2_1_4t(sc_module_name name, sc_module_name id_in1, sc_module_name id_in2, sc_module_name id_out)
    : smoc_actor(name, start), in1(id_in1), in2(id_in2), out(id_out)
  {
    start =(in1(4) && in2(1))   >>
           (out(1))  >>
           CALL(SimpleTask2_1_4t::process)  >>
           start;
  }

private:
  void process() {
    std::cout <<  name() << " recv/processed/sended " << in1[0]
              << " @ " << sc_time_stamp() << std::endl;
    out[0] = in1[0];
  }

  smoc_firing_state start;
};

