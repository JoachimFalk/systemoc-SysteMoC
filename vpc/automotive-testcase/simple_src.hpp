#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_tt.hpp>

class SimpleSrc: public smoc_periodic_actor{
public:
  smoc_port_out<int> out;
  
  SimpleSrc(sc_module_name name, sc_time per, sc_time off, float jitter=0.0)
    : smoc_periodic_actor(name, start, per, off, jitter)
  {
    count=0;
    start = out(1)                          >> 
      CALL(SimpleSrc::process)           >>
      start;
  }

private:
  int count;
  void process() {
  //  this->resetEvent();

    std::cout << name() << " send: " << count << "@ " << sc_time_stamp() << std::endl;
    out[0]=count++;
  }
  
  smoc_firing_state start;

};

