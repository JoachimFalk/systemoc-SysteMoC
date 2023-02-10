/*  Library : Math Operations
    Block : Sum, Add, Subtract
    Despcription : Add or subtract inputs
*/


#ifndef __INCLUDED__CONSTANT_HPP__
#define __INCLUDED__CONSTANT_HPP__

#include <cstdlib>
#include <iostream>


/*
You specify the operations of the block with the List of signs parameter. 
Plus (+), minus (-) characters indicate the operations to be performed on the inputs:
If there are two or more inputs, then the number of + and - characters must equal the number of inputs. 
For example, "+-+" requires three inputs and configures the block to subtract the second (middle) input 
from the first (top) input, and then add the third (bottom) input. 
*/

template<typename DATA_TYPE>
 class Constant: public smoc_actor {
public:
  smoc_port_out<DATA_TYPE>  out;

  Constant( sc_module_name name, DATA_TYPE constValue )
    : smoc_actor(name, start), constValue(constValue)
  {
    SMOC_REGISTER_CPARAM(constValue);

    start = out(1)     >>
      CALL(Constant::process) >> start
      ;
  }

protected:
  DATA_TYPE constValue;

  void process() {
      out[0] = constValue;
  }

  smoc_firing_state start;
};


#endif // __INCLUDED__CONSTANT_HPP__
