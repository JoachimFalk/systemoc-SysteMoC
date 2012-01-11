/*  Library : Discrete
    
*/


#ifndef __INCLUDED__DISCRETEDERIVATIVE__HPP__
#define __INCLUDED__DISCRETEDERIVATIVE__HPP__


#include <cstdlib>
#include <iostream>
#include <systemoc/smoc_moc.hpp>
//#include <actorlibrary/tt/TT.hpp>

template<typename T>
 class DiscreteDerivative: public smoc_actor {
public:
  smoc_port_in<T>  in;
  smoc_port_out<T>  out;

  DiscreteDerivative( sc_module_name name, T gain,T sampleTime,   T ic )
    : smoc_actor(name, start), gain(gain), sampleTime(sampleTime),  init(ic) {


    in_ = 0.0;

    start = in (1)     >>
      out(1)     >> 
      CALL(DiscreteDerivative::process) >> start
      ;
  }

protected:

  T gain;
  T sampleTime;
  T init;
  T in_; // block's input at the previous time step

  void process() {
	//std::cerr << this->name() << " : fired @ " << sc_time_stamp() << std::endl;
	 //step++;
         //std::cout << "Scope:<" << this->name() << "> " << " @ " << " state:" << state << " gain:" << gain << " sampleTime:" << sampleTime << std::endl;
         //printf ("in last: %5.6f, gain:%3.3f", in_, gain);
         out[0]=((in[0] - init )*gain)/sampleTime; 
         init = in[0];
         //printf (" Derivative: %5.6f, in: %5.6f \n", tmp, in[0]);
	//std::cerr << this->name() << " : Action finished" << std::endl;
  }

  smoc_firing_state start;
  //smoc_firing_state loop;
};

#endif // __INCLUDED__DISCRETEDERIVATIVE__HPP__

