/*  Library : Lookup Tables
    Block : Lookup
    Despcription : Approximate one-dimensional function y = f(x)
                   A lookup table block uses an array of data to map input values to output values, 
                   approximating a mathematical function. Given input values, the Simulink software 
                   performs a "lookup" operation to retrieve the corresponding output values from the 
                   table. If the lookup table does not define the input values, the block estimates the 
                   output values based on nearby table values.

    Note:The length of the x and y data vectors provided to this block must match.
         Also, the x data vector must be strictly monotonically increasing

    Note:The lookup method is Interpolation-Extrapolation (default)
         This default method performs linear interpolation and extrapolation of the inputs.

    TODO:
    Method: Interpolation-Use End Values
            Use Input Nearest
            Use Input Below
            Use Input Above
*/


#ifndef __INCLUDED__LOOKUP_P__HPP__
#define __INCLUDED__LOOKUP_P__HPP__
#include <cstdlib>
#include <iostream>
#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_tt.hpp>
//#include <actorlibrary/tt/TT.hpp>

template<typename T>
 class Lookup_p:  public smoc_periodic_actor {
public:
  smoc_port_in<T>  in;
  smoc_port_out<T>  out;

  Lookup_p( sc_module_name name,sc_time per, sc_time off, T* inputs, T* table, int length )
    : smoc_periodic_actor(name, start, per, off), inputs(inputs), table(table), length(length){


    start = //Expr::till( this->getEvent() )  >>
      out(1)     >> in (1)     >>
      CALL(Lookup_p::process) >> start
      ;
  }

protected:

  T* inputs;
  T* table;
  int length;
  T xData;
  T yData;
  
  void process() {
 	//this->resetEvent();
        int index=0;
	int boundary = 0;
        T xA;
        T xB;
        T yA;
        T yB; 

        xData = in[0];
	//std::cout << "Lookup> get: " << xData << " @ " << sc_time_stamp() << std::endl;
        for( index=0; index<length; index++ ){
           if( xData > inputs[index] )
              boundary = index;
	}

	// Linear Interpolation
        xA = inputs[boundary];
	xB = inputs[boundary+1];
	yA = table[boundary];
	yB = table[boundary+1];
        yData = ((yB-yA)/(xB-xA)) * (xData-xA) + yA;

	out[0] = yData;
  }

  smoc_firing_state start;
};

#endif // __INCLUDED__LOOKUP__HPP__

