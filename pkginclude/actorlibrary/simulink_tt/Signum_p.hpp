/*  Library : Math Operations
    Block : Signum
    Despcription : Indicate sign of input

    Description : The Sign block indicates the sign of the input
	          The output is 1 when the input is greater than zero
	          The output is 0 when the input is equal to zero
	          Then output is -1 when the input is less than zero 

TODO:Zero-Crossing
One:to detect when the input crosses through zero.
*/


#ifndef __INCLUDED__SIGNUM_P__HPP__
#define __INCLUDED__SIGNUM_P__HPP__


#include <cstdlib>
#include <iostream>
#include <systemoc/smoc_moc.hpp>
#include <actorlibrary/tt/TT.hpp>


template<typename T>
 class Signum_p: public PeriodicActor {
public:
  smoc_port_in<T>  in;
  smoc_port_out<T>  out;

  Signum_p( sc_module_name name, sc_time per, sc_time off, EventQueue* eventQueue )
    : PeriodicActor( name, start, per, off, eventQueue ){


    start =  Expr::till( this->getEvent() )  >>
      in(1)              >>
      out(1)             >>
      CALL(Signum_p::process) >> start
      ;
  }

protected:

  void process() {
	this->resetEvent();
	T data = in[0];
	if(  data == 0 )
	  out[0] = 0; 
	if(  data > 0 )
	  out[0] = 1;
	if(  data < 0 )
	  out[0] = -1; 
  }

  smoc_firing_state start;
};

#endif // __INCLUDED__SIGNUM_P__HPP__

