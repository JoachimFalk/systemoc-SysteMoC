#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_tt.hpp>

class AudioSink: public smoc_periodic_actor{
public:
  smoc_port_in<int> in;
  
  AudioSink(sc_module_name name, sc_time per, sc_time off)
    : smoc_periodic_actor(name, start, per, off)
  {
    
    start =(in(1))   >> 
      CALL(AudioSink::process)               >>
      start;

  }
  
private:
  void process() {
    std::cout <<  name() << " recv " << in[0]
              << " @ " << sc_time_stamp() << std::endl;
  }
  
  smoc_firing_state start;
};

