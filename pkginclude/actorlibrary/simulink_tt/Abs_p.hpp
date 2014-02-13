/*  Library : Math Operations
    Block : Abs
    Despcription : Output absolute value of input

 TODO: Zero-Crossing
*/


#ifndef __INCLUDED__ABS_P__HPP__
#define __INCLUDED__ABS_P__HPP__


#include <stdlib.h>

#include <cstdlib>
#include <iostream>
#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_tt.hpp>
//#include <actorlibrary/tt/TT.hpp>


template<typename T>
 class Abs_p:  public smoc_periodic_actor {
public:
  smoc_port_in<T>  in;
  smoc_port_out<T>  out;

  Abs_p( sc_module_name name, sc_time period, sc_time offset )
    : smoc_periodic_actor(name, start, period, offset){


    start =  //Expr::till( this->getEvent() )  >>
      in(1)                    >>
      out(1)                   >>
      CALL(Abs_p::process) >> start
      ;
  }

protected:

  void process() {
	 //this->resetEvent();
	 out[0] = abs((double)in[0]);
  }

  smoc_firing_state start;
};

#endif // __INCLUDED__ABS_P__HPP__

