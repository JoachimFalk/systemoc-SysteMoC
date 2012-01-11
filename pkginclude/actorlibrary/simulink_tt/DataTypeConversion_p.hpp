/*  Library : Signal Attributes
    Block : DataTypeConversion
    Despcription : Convert input signal to specified data type
                   The Data Type Conversion block converts an input signal of any 
                   Simulink software data type to the data type and scaling you specify 
                   for the Output data type parameter. The input can be any real- or 
                   complex-valued signal. If the input is real, the output is real. 
                   If the input is complex, the output is complex.
 
*/


#ifndef __INCLUDED__DATATYPECONVERSION_P__HPP__
#define __INCLUDED__DATATYPECONVERSION_P__HPP__

#include <cstdlib>
#include <iostream>
#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_tt.hpp>
//#include <actorlibrary/tt/TT.hpp>


template<typename T, typename S>
//template<typename T>
 class DataTypeConversion_p: public smoc_periodic_actor {
public:
  smoc_port_in<T>  in;
  smoc_port_out<S>  out;
//	smoc_port_out<T>  out;

  DataTypeConversion_p( sc_module_name name, sc_time per, sc_time off)
    : smoc_periodic_actor(name, start, per, off){


    start = //Expr::till( this->getEvent() )  >>
      out(1)     >> in (1)     >>
      CALL(DataTypeConversion_p::process) >> start
      ;
  }

protected:

  void process() {
	 //this->resetEvent();
	 out[0] = (S)(in[0]);
	//out[0] = in[0];
  }

  smoc_firing_state start;
};

#endif // __INCLUDED__DATATYPECONVERSION_P__HPP__

