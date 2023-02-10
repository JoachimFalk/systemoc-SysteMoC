/*  Dummy

The Signal Specification block allows you to specify the attributes of the signal connected to its input and output ports. If the specified attributes conflict with the attributes specified by the blocks connected to its ports, Simulink software displays an error when it compiles the model. For example, at the beginning of a simulation. If no conflict exists, Simulink eliminates the Signal Specification block from the compiled model. In other words, the Signal Specification block is a virtual block. It exists only to specify the attributes of a signal and plays no role in the simulation of the model.

*/


#ifndef __INCLUDED__SIGNALSPECIFICATION_P__HPP__
#define __INCLUDED__SIGNALSPECIFICATION_P__HPP__


#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_tt.hpp>
//#include <actorlibrary/tt/TT.hpp>

template<typename T>
 class SignalSpecification_p: public smoc_periodic_actor {
public:
  smoc_port_in<T>  in;
  smoc_port_out<T>  out;

  SignalSpecification_p( sc_module_name name, sc_time per, sc_time off)
    : smoc_periodic_actor(name, start, per, off)
  {


    start = //Expr::till( this->getEvent() )  >>
      out(1)     >> in (1)     >>
      CALL(SignalSpecification_p::process) >> start
      ;
  }

protected:

  //T previous_in;

  void process() {
         //this->resetEvent();
	 //std::cout << "Memory> storage: " << storage << " @ " << sc_time_stamp() << std::endl;
	 out[0] = in[0];
  }

  smoc_firing_state start;
};

#endif // __INCLUDED__SIGNALSPECIFICATION_p__HPP__

