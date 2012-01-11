/*  Dummy
*/


#ifndef __INCLUDED__GROUND_P__HPP__
#define __INCLUDED__GROUND_P__HPP__


#include <systemoc/smoc_moc.hpp>
#include <actorlibrary/tt/TT.hpp>

template<typename T>
 class Ground_p: public PeriodicActor {
public:
  smoc_port_out<T>  out;

  Ground_p( sc_module_name name, sc_time per, sc_time off, EventQueue* _eq )
    : PeriodicActor(name, start, per, off, _eq)
  {


    start = Expr::till( this->getEvent() )  >>
      out(1)      >>
      CALL(Ground_p::process) >> start
      ;
  }

protected:

  //T previous_in;

  void process() {
         this->resetEvent();
	 //std::cout << "Memory> storage: " << storage << " @ " << sc_time_stamp() << std::endl;
	 out[0] = 0;
  }

  smoc_firing_state start;
};

#endif // __INCLUDED__GROUND_p__HPP__

