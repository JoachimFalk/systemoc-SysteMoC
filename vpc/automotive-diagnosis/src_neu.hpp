#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_tt.hpp>
#include <systemoc/smoc_moc.hpp>
//#include "TT_neu.hpp"


class Source: public smoc_periodic_actor{
public:
  smoc_port_out<double> out;
  double tmp;
  
  Source(sc_module_name name, sc_time per, sc_time off)
    : smoc_periodic_actor(name, start, per, off)
  {
    tmp = 42.0;
    start = out(1)                          >> 
      CALL(Source::process)           >>
      start;
  }

private:


  void process() {
  //  this->resetEvent();

   // std::cout << "Source send: " << tmp << " @ "
   //           << sc_time_stamp() << std::endl;
    out[0]=tmp++;
  }
  
  smoc_firing_state start;

};

