#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_tt.hpp>

class SimpleTask_tt: public smoc_periodic_actor{
public:
  
  SimpleTask_tt(sc_module_name name, sc_time per, sc_time off, double jitter=0.0)
    : smoc_periodic_actor(name, start, per, off, jitter)
  {
    
    start = CALL(SimpleTask_tt::process)  >>
           start;
  }
  
private:
  void process() {

  }
  
  smoc_firing_state start;
};

