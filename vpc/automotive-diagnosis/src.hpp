#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_moc.hpp>
#include "TT.hpp"


class Source: public PeriodicActor{
public:
  smoc_port_out<double> out;
  double tmp;
  
  Source(sc_module_name name, sc_time per, sc_time off, EventQueue* _eq)
    : PeriodicActor(name, start, per, off, _eq)
  {
    tmp = 42.0;
    start = Expr::till( this->getEvent() )  >>
      out(1)                          >> 
      CALL(Source::process)           >>
      start;
  }

private:


  void process() {
    this->resetEvent();

   // std::cout << "Source send: " << tmp << " @ "
    //          << sc_time_stamp() << std::endl;
    out[0]=tmp++;
  }
  
  smoc_firing_state start;

};

