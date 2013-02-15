#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_tt.hpp>

class SimpleSource: public smoc_actor{
public:
  smoc_port_out<sc_time> out;
  
  SimpleSource(sc_module_name name)
    : smoc_actor(name, start)
  {
    start = out(1)                          >> 
      CALL(SimpleSource::process)           >>
      start;
  }

private:

  void process() {
  //  this->resetEvent();

  //  std::cout << name() << " send: " << sc_time_stamp() << std::endl;
    out[0]=sc_time_stamp();
  }
  
  smoc_firing_state start;

};

