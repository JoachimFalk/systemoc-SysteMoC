#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_tt.hpp>
//#include "TT_neu.hpp"


class Sink: public smoc_periodic_actor{
public:
  smoc_port_in<double> in;
  
  Sink(sc_module_name name, sc_time per, sc_time off)
    : smoc_periodic_actor(name, start, per, off)
  {
    
    start =(in(1))   >> 
      CALL(Sink::process)               >>
      start;

  }
  
private:


  void process() {
//    this->resetEvent();

   // std::cout << "Sink recv " << in[0]
   //           << " @ " << sc_time_stamp() << std::endl;
  }
  
  smoc_firing_state start;
};

