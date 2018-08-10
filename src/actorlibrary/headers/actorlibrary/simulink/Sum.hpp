/*  Library : Math Operations
    Block : Sum, Add, Subtract
    Despcription : Add or subtract inputs
*/
#ifndef __INCLUDED__SUM__HPP__
#define __INCLUDED__SUM__HPP__

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
#include <iostream>
#include <systemoc/smoc_expr.hpp>
//enum OPERATOR {PLUS, MINUS};

template<typename DATA_TYPE, int PORTS=1>
class Sum: public smoc_actor {
public:

  smoc_port_in<DATA_TYPE>   in[PORTS];
  smoc_port_out<DATA_TYPE>  out;	
  
  Sum( sc_module_name name, std::string operators )
    : smoc_actor(name, start), operators(operators) {

    SMOC_REGISTER_CPARAM(operators);

    Expr::Ex<bool >::type eIn(in[0](1) );

    for(int i = 1; i < PORTS; i++){
      eIn = eIn && in[i](1);
    }

    start = eIn  >> out(1)                  >> 
      CALL(Sum::sum) >> start
      ;
  }
protected:
 // const OPERATOR * operators;
  std::string operators;

  void sum() {   
    
    
#ifdef _DEBUG    
    cout << name();
#endif    
    DATA_TYPE output= 0.0;
    
    for( int i = 0; i<PORTS; i++ ){
      
      DATA_TYPE tmp = in[i][0];
#ifdef _DEBUG      
      cout << " " << tmp;
#endif
      
      if( operators[i] == '+' ){
        output = output + tmp;
      }else if( operators[i] == '-' ){
        output = output - tmp;
      }
    }
    out[0] = output;
#ifdef _DEBUG    
    cout << " " << output << endl;
#endif
  }

  smoc_firing_state start;

};

#endif// __INCLUDED__SUM__HPP__

