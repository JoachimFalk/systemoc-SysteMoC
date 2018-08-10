/*  Library : Math Operations
    Block : Sum, Add, Subtract
    Despcription : Add or subtract inputs
*/
#ifndef __INCLUDED__SUM_P__HPP__
#define __INCLUDED__SUM_P__HPP__

/*
You specify the operations of the block with the List of signs parameter. 
Plus (+), minus (-) characters indicate the operations to be performed on the
inputs:
If there are two or more inputs, then the number of + and - characters must
equal the number of inputs. 
For example, "+-+" requires three inputs and configures the block to subtract
the second (middle) input 
from the first (top) input, and then add the third (bottom) input. 
*/
#include <cstdlib>
#include <iostream>


#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_tt.hpp>
#include <systemoc/smoc_expr.hpp>
//#include <actorlibrary/tt/TT.hpp>
//enum OPERATOR {PLUS, MINUS};

template<typename DATA_TYPE, int PORTS=1>
 class Sum_p: public smoc_periodic_actor {
public:

  smoc_port_in<DATA_TYPE>   in[PORTS];
  smoc_port_out<DATA_TYPE>  out;	
  
  Sum_p( sc_module_name name, sc_time per, sc_time off, std::string operators )
    : smoc_periodic_actor(name, start, per, off), operators(operators) {

    Expr::Ex<bool >::type eIn(in[0](1) );

    for(int i = 1; i < PORTS; i++){
      eIn = eIn && in[i](1);
    }

    start = //Expr::till( this->getEvent() )  >>
      out(1) >> eIn                    >> 
      CALL(Sum_p::sum) >> start
      ;
  }
protected:
 // const OPERATOR * operators;
  std::string operators;

  void sum() {   
	//this->resetEvent();

          

    DATA_TYPE output=0.0;
	
    for( int i = 0; i<PORTS; i++ ){
      DATA_TYPE tmp = in[i][0];
      	#ifdef EnablePrint
	std::cout << sc_time_stamp() << " " <<  name() << " " << " in[" << i << "]=" << tmp << "\n";
	#endif
      if( operators[i] == '+' ){
        output = output + tmp;
      }else if( operators[i] == '-' ){
        output = output - tmp;
      }else{
        assert(0);
      }
    }
  
//std::cout << "Sum> output: " << output << " @ " << sc_time_stamp() << std::endl;
//std::cout << "Sum> " << this->name() << std::endl;
/*
std::cout << "Sum> get: " << in[0][0] << " @ " << sc_time_stamp()
                << std::endl;
std::cout << "Sum> get: " << in[1][0] << " @ " << sc_time_stamp()
                << std::endl;
std::cout << "Sum> output: " << output << " @ " << sc_time_stamp()
                << std::endl;

*/
    out[0] = output;//output;
    
	#ifdef EnablePrint
	std::cout << sc_time_stamp() << " " <<  name() << " " << " out : " << output << "\n";
	#endif
  }

  smoc_firing_state start;

};

#endif// __INCLUDED__SUM_P__HPP__

