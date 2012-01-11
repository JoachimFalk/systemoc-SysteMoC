/*  Library : Math Operations
    Block : Abs
    Despcription : Output absolute value of input

 TODO: Zero-Crossing
*/


#ifndef __INCLUDED__ABS__HPP__
#define __INCLUDED__ABS__HPP__

#include <stdlib.h>


template<typename T>
 class Abs: public smoc_actor {
public:
  smoc_port_in<T>  in;
  smoc_port_out<T>  out;

  Abs( sc_module_name name )
    : smoc_actor(name, start){


    start = in(1)              >>
      out(1)                   >>
      CALL(Abs::process) >> start
      ;
  }

protected:

  void process() {
	 out[0] = abs(in[0]);
  }

  smoc_firing_state start;
};

#endif // __INCLUDED__ABS__HPP__

