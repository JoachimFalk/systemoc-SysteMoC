/*  Library : Signal Attributes
    Block : DataTypeConversion
    Despcription : Convert input signal to specified data type
                   The Data Type Conversion block converts an input signal of any 
                   Simulink software data type to the data type and scaling you specify 
                   for the Output data type parameter. The input can be any real- or 
                   complex-valued signal. If the input is real, the output is real. 
                   If the input is complex, the output is complex.
 
*/


#ifndef __INCLUDED__DATATYPECONVERSION__HPP__
#define __INCLUDED__DATATYPECONVERSION__HPP__

#include <stdlib.h>

#include <cstdlib>
#include <iostream>
#include <systemoc/smoc_moc.hpp>
template<typename T, typename S>
 class DataTypeConversion: public smoc_actor {
public:
  smoc_port_in<T>  in;
  smoc_port_out<S>  out;

  DataTypeConversion( sc_module_name name )
    : smoc_actor(name, start){


    start = in(1)              >>
      out(1)                   >>
      CALL(DataTypeConversion::process) >> start
      ;
  }

protected:

  void process() {
	 out[0] = (S)(in[0]);
  }

  smoc_firing_state start;
};

#endif // __INCLUDED__DATATYPECONVERSION__HPP__

