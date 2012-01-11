/*  Library : Math Operations
    Block : MinMax
    Despcription : Output the minimum or maximum input value

    TODO: Zero-Crossing
*/



#ifndef __INCLUDED__MINMAX__HPP__
#define __INCLUDED__MINMAX__HPP__

#include <cstdlib>
#include <algorithm>

#include <iostream>

#include <systemoc/smoc_moc.hpp>
#include <actorlibrary/tt/TT.hpp>
/*
A MinMax block accepts and outputs real-valued signals of any data type except int64 and uint64 
*/

template<typename T, int INPUTPORTS=1>

 class MinMax_p: public PeriodicActor {
public:
  smoc_port_in<T>   in[INPUTPORTS];
  smoc_port_out<T>  out;	

  MinMax_p( sc_module_name name, sc_time period, sc_time offset, EventQueue* eventQueue, int function )
    : PeriodicActor(name, start, period, offset, eventQueue), function(function) {

    Expr::Ex<bool >::type eIn(in[0](1) );

    for(int i = 1; i < INPUTPORTS; i++){
      eIn = eIn && in[i](1);
    }

    start = Expr::till( this->getEvent() ) >>
      out(1) 		>>
      eIn                    >> 
      CALL(MinMax_p::minmax) >> start
      ;
  }
protected:
 
  

  /* 
    operator '<' meaning function MIN
    operator '>' meaning function MAX	
   */
  int function;
  

  void minmax() {   
	this->resetEvent();
	T output = (T)0;

	
	
		
	for( int i=0; i<INPUTPORTS; i++ ){
	  //if( function == '<' )
	  if( function == 1 )
		output = std::min( output, in[i][0] );
		//output = ( output < in[i][0] ) ? output : in[i][0];
	  //if( function == '>' )
          if( function == 2 )	  
		output = std::max( output, in[i][0] );
		//output = ( output > in[i][0] ) ? in[i][0] : output;
		//output = 2.3;
          else
	        //output = std::min(output, in[i][0]);
		output = ( output < in[i][0] ) ? output : in[i][0];
	}
	out[0] = output;
	
	/*
	std::cout << "MinMax> output " <<  (( output < in[1][0] ) ? output : in[1][0]) << " @ " << sc_time_stamp() << std::endl;
	std::cout << "MinMax> get " << in[0][0] << " @ " << sc_time_stamp() << std::endl;

	std::cout << "MinMax> get " << in[1][0] << " @ " << sc_time_stamp() << std::endl;
	std::cout << "MinMax> inputs " << INPUTPORTS << " @ " << sc_time_stamp() << std::endl;
	std::cout << "MinMax> function: " << function << " @ " << sc_time_stamp() << std::endl;
	*/
  }

  smoc_firing_state start;


};

#endif // __INCLUDED__MINMAX__HPP__
