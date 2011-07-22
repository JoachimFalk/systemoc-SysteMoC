#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_tt.hpp>

class SimpleActor_tt: public smoc_periodic_actor{
public:

  SimpleActor_tt(sc_module_name name, sc_time per, sc_time off, float jitter=0.0)
    : smoc_periodic_actor(name, start, per, off, jitter)
  {

    start =  CALL(SimpleActor_tt::process)  >> start;
  }

private:
  void process() {
    //std::cout << name() << "simpleActor_tt executed " << " @ " << sc_time_stamp() << std::endl;

  }


  smoc_firing_state start;
};

