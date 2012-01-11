/*  Library : IO adapter
    Block : Inport, Outport
    Despcription : IO adapter for subsystem's inport and outport
*/


#ifndef __INCLUDED__IOADAPTER_P__HPP__
#define __INCLUDED__IOADAPTER_P__HPP__

#include <cstdlib>
#include <iostream>
#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_tt.hpp>
//#include <actorlibrary/tt/TT.hpp>


template<typename T>
 class IOAdapter_p: public smoc_periodic_actor {
public:
  smoc_port_in<T>  in;
  smoc_port_out<T>  out;

  IOAdapter_p( sc_module_name name, sc_time per, sc_time off, T _init )
    : smoc_periodic_actor(name, start, per, off),init(_init) {


    start = //Expr::till( this->getEvent() )  >>
      out(1)     >> in (1)     >>
      CALL(IOAdapter_p::process) >> start
      ;
  }

protected:

  void process() {
 	//this->resetEvent();

	if( init != (T)0 )
 	{
		out[0] = init;
	        init = (T)0;
	}
	 
	else
	 	out[0] = in[0];
  }

  smoc_firing_state start;
  T init;
};

#endif // __INCLUDED__IOADAPTER_P__HPP__

