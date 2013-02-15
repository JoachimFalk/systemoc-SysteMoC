#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_moc.hpp>
#include "TT.hpp"


class Sink: public PeriodicActor{
public:
  smoc_port_in<double> in;
  
  Sink(sc_module_name name, sc_time per, sc_time off, EventQueue* _eq)
    : PeriodicActor(name, start, per, off, _eq)
  {
    
    start = ( Expr::till( this->getEvent() )) >>
      (in(1))   >> 
      CALL(Sink::process)               >>
      start;

  }
  
private:


  void process() {
    this->resetEvent();

   // std::cout << "Sink recv " << in[0]
   //           << " @ " << sc_time_stamp() << std::endl;
  }
  
  smoc_firing_state start;
};

