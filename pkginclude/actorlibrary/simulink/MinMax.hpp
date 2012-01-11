/*  Library : Math Operations
    Block : MinMax
    Despcription : Output the minimum or maximum input value

    TODO: Zero-Crossing
*/



#ifndef __INCLUDED__MINMAX_P__HPP__
#define __INCLUDED__MINMAX_P__HPP__

#include <cstdlib>
#include <algorithm>

#include <iostream>

#include <systemoc/smoc_moc.hpp>
//#include <actorlibrary/tt/TT.hpp>
/*
A MinMax block accepts and outputs real-valued signals of any data type except int64 and uint64 
*/

template<typename T, int INPUTPORTS=1>

 class MinMax: public smoc_actor {
public:
  smoc_port_in<T>   in[INPUTPORTS];
  smoc_port_out<T>  out;	
protected:
 
  

  /* 
    operator '<' meaning function MIN
    operator '>' meaning function MAX	
   */
  char function;
  

  void minmax() {   
	
	T output;
		
	for( int i=0; i<=INPUTPORTS; i++ ){
	  if( function == '<' )
		output = std::min( output, in[i][0] );
	  if( function == '>' )
		output = std::max( output, in[i][0] );
	}
	out[0] = output;
  }

  smoc_firing_state start;

public:
  MinMax( sc_module_name name, char function )
    : smoc_actor(name, start), function(function) {

    Expr::Ex<bool >::type eIn(in[0](1) );

    for(int i = 1; i < INPUTPORTS; i++){
      eIn = eIn && in[i](1);
    }

    start = eIn                    >> 
      CALL(MinMax::minmax) >> start
      ;
  }
};

#endif // __INCLUDED__MINMAX__HPP__
